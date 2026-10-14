#include "CmsHelpers.h"
#include "CmsHelpers.cxx"
#include "style/tdrstyle.h"
#include "style/tdrstyle.cxx"
#include "style/CMS_lumi.h"
#include "style/CMS_lumi.cxx"
#include "MuographyAnalysis.h"
#include "MuographyAnalysis.cxx"
#include "DiffMap.h"
#include "DiffMap.cxx"
#include "EfficiencySummary.h"
#include "EfficiencySummary.cxx"

#include <stdio.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <set>

#include "TROOT.h"
#include "TFile.h"
#include "TKey.h"
#include "TH1F.h"
#include "TCanvas.h"

void muogr_v4(const char *fileRef,
              const char *fileComp,
              const char *histoPath,
              const char *year1 = "2018",
              const char *year2 = "2025",
              const char *rollNamesFile = "rollNames.txt",
              const char *allRolls = "",
              const char *gridTemplateFile = "files/SummaryAnalyzeEfficiency_402653_RPCMon2026.root")
{
    setTDRStyle();

    const std::string outDir = "for_conference_2026/";
    const std::string sYear1 = std::string(year1);
    const std::string sYear2 = std::string(year2);
    const std::string sVsTag = sYear1 + "vs" + sYear2;
    const bool processAllRolls = std::string(allRolls).length() > 0;

    if (gSystem->AccessPathName(outDir.c_str()))
        gSystem->mkdir(outDir.c_str(), kTRUE);

    if (gSystem->AccessPathName(fileRef))
    {
        std::cerr << "ERROR: reference file not found: " << fileRef << std::endl;
        return;
    }
    if (gSystem->AccessPathName(fileComp))
    {
        std::cerr << "ERROR: comparison file not found: " << fileComp << std::endl;
        return;
    }

    std::set<std::string> targetChambers;
    if (!processAllRolls)
    {
        if (gSystem->AccessPathName(rollNamesFile))
        {
            std::cerr << "ERROR: roll names file not found: " << rollNamesFile << std::endl;
            return;
        }

        targetChambers = loadRollNames(rollNamesFile);
        if (targetChambers.empty())
        {
            std::cerr << "ERROR: no roll names loaded from " << rollNamesFile << std::endl;
            return;
        }
    }

    TDirectory *dirR = openFileAtPath(fileRef, histoPath);
    if (!dirR)
        return;
    TDirectory *dirC = openFileAtPath(fileComp, histoPath);
    if (!dirC)
        return;

    TCanvas *cR = new TCanvas("cR", "cR", 558, 409, 900, 600);
    TCanvas *cC = new TCanvas("cC", "cC", 558, 409, 900, 600);
    TCanvas *cDiff = new TCanvas("cDiff", "cDiff", 558, 409, 900, 600);

    DecorateFn decorate = [](TCanvas *c, const std::string &rightText)
    {
        CMS_lumi(c, rightText);
    };

    EfficiencySummary *effSummary = nullptr;
    if (gridTemplateFile && strlen(gridTemplateFile) > 0)
    {
        if (gSystem->AccessPathName(gridTemplateFile))
        {
            std::cerr << "WARNING: efficiency-grid template not found, skipping grid summary: "
                      << gridTemplateFile << std::endl;
        }
        else
        {
            effSummary = new EfficiencySummary(gridTemplateFile);
            if (!effSummary->isValid())
            {
                std::cerr << "WARNING: efficiency-grid template had no usable histograms, skipping grid summary"
                          << std::endl;
                delete effSummary;
                effSummary = nullptr;
            }
        }
    }

    TH1F *hmyAssymetry = createAsymmetryHistogram(sYear1, sYear2);

    const std::string meansOutPath = outDir + "relative_diff_means_" + sVsTag + ".txt";
    std::ofstream outMeansTxt(meansOutPath.c_str());
    if (!outMeansTxt.is_open())
        std::cerr << "ERROR: could not create means output file: " << meansOutPath << std::endl;
    outMeansTxt << "# rollName  meanRelDiff(" << sYear1 << "," << sYear2 << ")" << std::endl;

    const std::string combinedOutPath = outDir + "all_results_" + sVsTag + ".root";
    TFile *combinedOutFile = TFile::Open(combinedOutPath.c_str(), "RECREATE");
    if (!combinedOutFile || combinedOutFile->IsZombie())
    {
        std::cerr << "ERROR: could not create combined ROOT file: " << combinedOutPath << std::endl;
        combinedOutFile = nullptr;
    }

    TIter iterR(dirR->GetListOfKeys());
    TKey *keyR;
    int myCount = 0;
    while ((keyR = (TKey *)iterR.Next()))
    {
        myCount++;
        if (!processAllRolls && targetChambers.find(keyR->GetName()) == targetChambers.end())
            continue;

        processRoll(keyR, dirC, outDir, sYear1, sYear2, cR, cC, cDiff, hmyAssymetry, outMeansTxt, combinedOutFile, decorate, effSummary);
    }

    outMeansTxt.close();
    std::cout << "Per-roll mean relative differences written to " << meansOutPath << std::endl;

    if (effSummary)
    {
        const std::string gridOutPath = outDir + "relative_efficiency_summary_" + sVsTag + ".root";
        effSummary->save(gridOutPath);
        delete effSummary;
        std::cout << "Relative efficiency summary grid written to " << gridOutPath << std::endl;
    }

    if (combinedOutFile)
    {
        combinedOutFile->cd();
        hmyAssymetry->Write();
        combinedOutFile->Write();
        combinedOutFile->Close();
        delete combinedOutFile;
        std::cout << "Combined results written to " << combinedOutPath << std::endl;
    }

    dirR->GetFile()->Close();
    dirC->GetFile()->Close();

    delete hmyAssymetry;
    delete cR;
    delete cC;
    delete cDiff;
}
