#include "CmsHelpers.h"
#include <string>
#include <algorithm>
#include <set>

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