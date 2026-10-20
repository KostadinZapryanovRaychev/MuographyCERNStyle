#include "RelDiffByWheelDisk.h"

#include <sstream>
#include <vector>

#include "TH1.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TFile.h"

// Note: these helpers are named distinctly from the near-identical ones in
// EfficiencySummary.cxx (splitByUnderscore/parseWheel/parseDisk) even
// though both are in anonymous namespaces — muogr_v4.cxx #includes both
// .cxx files textually into one translation unit, where every anonymous
// namespace in a TU is the SAME namespace, so identically-named helpers
// would collide at link time.
namespace
{

std::vector<std::string> rdSplitByUnderscore(const std::string &s)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, '_'))
        tokens.push_back(tok);
    return tokens;
}

// "W-2" -> -2, "W+1" -> 1, "W0" -> 0
int rdParseWheel(const std::string &tok)
{
    return std::stoi(tok.substr(1));
}

// "RE-1" -> -1, "RE+2" -> 2
int rdParseDisk(const std::string &tok)
{
    return std::stoi(tok.substr(2));
}

} // namespace

RelDiffByWheelDisk::RelDiffByWheelDisk(const std::string &sYear1, const std::string &sYear2)
    : sYear1_(sYear1), sYear2_(sYear2)
{
    std::string titleBase =
        "(Eff(" + sYear1 + ")-Eff(" + sYear2 + "))/(Eff(" + sYear1 + ")+Eff(" + sYear2 + "))";

    for (int wheel = -2; wheel <= 2; wheel++)
    {
        std::string name = "RelDiff1D_W" + std::to_string(wheel);
        TH1F *h = new TH1F(name.c_str(), (titleBase + " Wheel " + std::to_string(wheel)).c_str(), 201, -2., 2.);
        h->SetDirectory(nullptr);
        barrel_[wheel] = h;
    }

    for (int disk = -4; disk <= 4; disk++)
    {
        if (disk == 0)
            continue;
        std::string name = "RelDiff1D_D" + std::to_string(disk);
        TH1F *h = new TH1F(name.c_str(), (titleBase + " Disk " + std::to_string(disk)).c_str(), 201, -2., 2.);
        h->SetDirectory(nullptr);
        endcap_[disk] = h;
    }
}

RelDiffByWheelDisk::~RelDiffByWheelDisk()
{
    for (auto &kv : barrel_)
        delete kv.second;
    for (auto &kv : endcap_)
        delete kv.second;
}

void RelDiffByWheelDisk::addRoll(const std::string &rawRollName, TH1F *rollRelDiff1D)
{
    std::vector<std::string> tokens = rdSplitByUnderscore(rawRollName);
    if (tokens.size() < 2 || tokens[0] != "Muography")
        return;

    if (!tokens[1].empty() && tokens[1][0] == 'W')
    {
        auto it = barrel_.find(rdParseWheel(tokens[1]));
        if (it != barrel_.end())
            it->second->Add(rollRelDiff1D);
    }
    else if (tokens[1].size() > 1 && tokens[1][0] == 'R' && tokens[1][1] == 'E')
    {
        auto it = endcap_.find(rdParseDisk(tokens[1]));
        if (it != endcap_.end())
            it->second->Add(rollRelDiff1D);
    }
}

void RelDiffByWheelDisk::save(const std::string &outDir, TFile *outFile, TCanvas *c, const DecorateFn &decorate)
{
    for (auto &kv : barrel_)
    {
        std::string label = "Wheel " + std::to_string(kv.first);
        drawRelDiff1D(c, kv.second);
        decorate(c, label + "  " + sYear1_ + " vs " + sYear2_);
        c->Modified();
        c->Update();
        std::string tag = std::string(kv.second->GetName()) + "_" + sYear1_ + "vs" + sYear2_;
        c->SaveAs((outDir + tag + ".png").c_str());
        c->SaveAs((outDir + tag + ".root").c_str());
        saveCanvasToCombinedRoot(c, outFile, tag);
    }

    for (auto &kv : endcap_)
    {
        std::string label = "Disk " + std::to_string(kv.first);
        drawRelDiff1D(c, kv.second);
        decorate(c, label + "  " + sYear1_ + " vs " + sYear2_);
        c->Modified();
        c->Update();
        std::string tag = std::string(kv.second->GetName()) + "_" + sYear1_ + "vs" + sYear2_;
        c->SaveAs((outDir + tag + ".png").c_str());
        c->SaveAs((outDir + tag + ".root").c_str());
        saveCanvasToCombinedRoot(c, outFile, tag);
    }
}
