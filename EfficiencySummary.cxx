#include "EfficiencySummary.h"

#include <iostream>
#include <sstream>

#include "TFile.h"
#include "TH2.h"
#include "TDirectory.h"

namespace
{

// The signed relative difference (aR - aC) / (aR + aC) is bounded by
// [-1, +1] in theory, but real per-roll values cluster tightly around 0
// (typically within +-0.15) — only a couple of bad Gaussian fits ever
// reach the extremes. Scaling the palette to +-1 crushes all real signal
// into an invisible sliver near white; +-0.15 shows the real spread while
// still clamping (and thereby flagging) those rare bad-fit outliers to
// full red/green.
const double kGridScaleLimit = 0.15;

std::vector<std::string> splitByUnderscore(const std::string &s)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, '_'))
        tokens.push_back(tok);
    return tokens;
}

// "W-2" -> -2, "W+1" -> 1, "W0" -> 0
int parseWheel(const std::string &tok)
{
    return std::stoi(tok.substr(1));
}

// "S07" -> 7
int parseSector(const std::string &tok)
{
    return std::stoi(tok.substr(1));
}

// "Backward"/"Forward"/"Middle" -> "_B"/"_F"/"_M", empty if unrecognized
std::string sideSuffix(const std::string &side)
{
    if (side == "Backward")
        return "_B";
    if (side == "Forward")
        return "_F";
    if (side == "Middle")
        return "_M";
    return "";
}

// "RE-1" -> -1, "RE+2" -> 2
int parseDisk(const std::string &tok)
{
    return std::stoi(tok.substr(2));
}

// "R2" -> 2, "R3" -> 3
int parseRing(const std::string &tok)
{
    return std::stoi(tok.substr(1));
}

// "CH01" -> 1
int parseChamber(const std::string &tok)
{
    return std::stoi(tok.substr(2));
}

} // namespace

EfficiencySummary::EfficiencySummary(const std::string &templateFile)
{
    TFile *f = TFile::Open(templateFile.c_str());
    if (!f || f->IsZombie())
    {
        std::cerr << "ERROR: could not open efficiency-grid template: " << templateFile << std::endl;
        return;
    }

    for (int wheel = -2; wheel <= 2; wheel++)
    {
        std::string name = "Barrel/FiducialCutEffW" + std::to_string(wheel);
        TH2F *src = (TH2F *)f->Get(name.c_str());
        if (!src)
            continue;

        TH2F *h = (TH2F *)src->Clone(("FiducialCutEffW" + std::to_string(wheel)).c_str());
        h->Reset();
        h->SetDirectory(nullptr);
        // Signed relative difference: negative = efficiency improved
        // (good), 0 = no change (neutral), positive = efficiency dropped
        // (bad). See kGridScaleLimit for why the range isn't +-1.
        h->SetMinimum(-kGridScaleLimit);
        h->SetMaximum(kGridScaleLimit);
        barrel_[wheel] = h;
    }

    for (int disk = -4; disk <= 4; disk++)
    {
        if (disk == 0)
            continue;
        std::string name = "Endcap/FiducialCutEffD" + std::to_string(disk);
        TH2F *src = (TH2F *)f->Get(name.c_str());
        if (!src)
            continue;

        TH2F *h = (TH2F *)src->Clone(("FiducialCutEffD" + std::to_string(disk)).c_str());
        h->Reset();
        h->SetDirectory(nullptr);
        h->SetMinimum(-kGridScaleLimit);
        h->SetMaximum(kGridScaleLimit);
        endcap_[disk] = h;
    }

    f->Close();
    delete f;
}

EfficiencySummary::~EfficiencySummary()
{
    for (auto &kv : barrel_)
        delete kv.second;
    for (auto &kv : endcap_)
        delete kv.second;
}

bool EfficiencySummary::isValid() const
{
    return !barrel_.empty() || !endcap_.empty();
}

void EfficiencySummary::fillBarrelRoll(const std::vector<std::string> &tokens, double value)
{
    // tokens: ["Muography", "W<wheel>", "<station>", "S<sector>", "<side>"]
    if (tokens.size() != 5)
        return;

    int wheel = parseWheel(tokens[1]);
    auto it = barrel_.find(wheel);
    if (it == barrel_.end())
        return;
    TH2F *h = it->second;

    int sector = parseSector(tokens[3]);
    std::string suffix = sideSuffix(tokens[4]);
    if (suffix.empty())
        return;

    std::string station = tokens[2];
    if (station == "RB4-")
        station = "RB4,-"; // matches the template's (odd but real) axis label
    std::string label = station + suffix;

    int nY = h->GetNbinsY();
    for (int y = 1; y <= nY; y++)
    {
        if (label == h->GetYaxis()->GetBinLabel(y))
        {
            h->SetBinContent(sector, y, value);
            return;
        }
    }
}

void EfficiencySummary::fillEndcapRoll(const std::vector<std::string> &tokens, double value)
{
    // tokens: ["Muography", "RE<disk>", "R<ring>", "CH<chamber>", "<segment>"]
    if (tokens.size() != 5)
        return;

    int disk = parseDisk(tokens[1]);
    auto it = endcap_.find(disk);
    if (it == endcap_.end())
        return;
    TH2F *h = it->second;

    int ring = parseRing(tokens[2]);
    int chamber = parseChamber(tokens[3]);
    const std::string &segment = tokens[4];

    int base;
    if (ring == 2)
        base = 0;
    else if (ring == 3)
        base = 3;
    else
        return;

    int offset;
    if (segment == "C")
        offset = 0;
    else if (segment == "B")
        offset = 1;
    else if (segment == "A")
        offset = 2;
    else
        return;

    int y = base + offset + 1;
    h->SetBinContent(chamber, y, value);
}

void EfficiencySummary::fillRoll(const std::string &rawRollName, double value)
{
    std::vector<std::string> tokens = splitByUnderscore(rawRollName);
    if (tokens.empty() || tokens[0] != "Muography" || tokens.size() < 2)
        return;

    // Signed relative difference (aR - aC) / (aR + aC): negative means
    // year2 was more efficient than year1 (improvement, good), positive
    // means year1 was more efficient (efficiency dropped, bad).
    if (tokens[1].size() > 0 && tokens[1][0] == 'W')
        fillBarrelRoll(tokens, value);
    else if (tokens[1].size() > 1 && tokens[1][0] == 'R' && tokens[1][1] == 'E')
        fillEndcapRoll(tokens, value);
}

void EfficiencySummary::save(const std::string &outPath) const
{
    TFile *f = TFile::Open(outPath.c_str(), "RECREATE");
    if (!f || f->IsZombie())
    {
        std::cerr << "ERROR: could not create efficiency-grid summary file: " << outPath << std::endl;
        return;
    }

    TDirectory *barrelDir = f->mkdir("Barrel");
    barrelDir->cd();
    for (auto &kv : barrel_)
        kv.second->Write();

    TDirectory *endcapDir = f->mkdir("Endcap");
    endcapDir->cd();
    for (auto &kv : endcap_)
        kv.second->Write();

    f->Close();
    delete f;
}
