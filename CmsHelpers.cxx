#include "CmsHelpers.h"
#include "TLatex.h"
#include <string>
#include <algorithm>
#include <set>

void drawCMSHeader(const std::string &label,
                   const std::string &energy)
{
    double y = 0.92;
    double xLeft = 0.05;

    TLatex cms;
    cms.SetNDC();
    cms.SetTextFont(61);
    cms.SetTextSize(0.05);
    cms.DrawLatex(xLeft, y, "CMS");

    TLatex prelim;
    prelim.SetNDC();
    prelim.SetTextFont(52);
    prelim.SetTextSize(0.045);
    prelim.DrawLatex(xLeft + 0.07, y, "Preliminary");

    TLatex energyTxt;
    energyTxt.SetNDC();
    energyTxt.SetTextFont(42);
    energyTxt.SetTextSize(0.04);
    energyTxt.DrawLatex(xLeft + 0.72, y, energy.c_str());

    TLatex ch;
    ch.SetNDC();
    ch.SetTextFont(42);
    ch.SetTextSize(0.04);
    ch.SetTextAlign(22);
    ch.DrawLatex(0.45, 0.93, label.c_str());
}

void drawCMSPreliminaryOverlay(
    const std::string &yearLabel,
    double xCMS,
    double xPre,
    double xData,
    double y)
{
    TLatex cms;
    cms.SetNDC();
    cms.SetTextFont(61);
    cms.SetTextSize(0.05);
    cms.DrawLatex(xCMS, y, "CMS");

    TLatex prelim;
    prelim.SetNDC();
    prelim.SetTextFont(52);
    prelim.SetTextSize(0.04);
    prelim.DrawLatex(xPre, y, "Preliminary");

    TLatex data;
    data.SetNDC();
    data.SetTextFont(42);
    data.SetTextSize(0.035);
    data.DrawLatex(xData, y - 0.06, yearLabel.c_str());

    TLatex energy;
    energy.SetNDC();
    energy.SetTextFont(42);
    energy.SetTextSize(0.04);
    energy.SetTextAlign(32);
    energy.DrawLatex(0.90, y, "(13 TeV - 13.6 TeV)");
}

std::string getEnergyLabel(const std::string &year)
{
    if (year == "2018")
        return "2018 data (13 TeV)";

    if (year == "2025")
        return "2025 data (13.6 TeV)";

    return year + " TeV";
}

std::string sanitizeName(const std::string &name)
{
    std::string s = name;
    std::replace(s.begin(), s.end(), '-', 'M');
    std::replace(s.begin(), s.end(), '+', 'P');
    return s;
}

std::string extractChamberName(const std::string &histoName)
{
    auto pos = histoName.find('_');

    if (pos == std::string::npos)
        return histoName;

    std::string out = histoName.substr(pos + 1);

    std::replace(out.begin(), out.end(), '-', 'M');

    return out;
}

std::set<std::string> loadRollNames(const char *rollNamesFile)
{
    std::set<std::string> chambers;
    std::ifstream ifs(rollNamesFile);
    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        chambers.insert(line);
    }
    ifs.close();
    return chambers;
}

TDirectory *openFileAtPath(const char *filePath, const char *histoPath)
{
    TFile *f = TFile::Open(filePath);
    if (!f || f->IsZombie())
    {
        std::cerr << "ERROR: could not open file: " << filePath << std::endl;
        return nullptr;
    }
    if (!f->cd(histoPath))
    {
        std::cerr << "ERROR: path '" << histoPath << "' not found in: " << filePath << std::endl;
        f->Close();
        return nullptr;
    }
    return gDirectory;
}

std::string buildAsymmetryTitle(const std::string &sYear1,
                                const std::string &sYear2)
{
    return "Relative asymmetry Eff(" + sYear1 + ") vs Eff(" + sYear2 + ")";
}