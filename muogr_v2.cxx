#include <stdio.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <set>
#include "TROOT.h"
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TH1.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "TCanvas.h"

/**
 * fileRef      : path to the reference ROOT file
 * fileComp     : path to the comparison ROOT file
 * histoPath    : internal directory inside each file (e.g. "Muogr")
 * year1        : label for the reference file  (e.g. "2018")
 * year2        : label for the comparison file (e.g. "2025")
 * rollNamesFile : path to a plain text file with one roll name per line
 *
 * Execution examples:
 * [] .x muogr_v2.cxx("AnalyzeEfficiency_321475_325172_RPCMon_2018D.root", "AnalyzeEfficiency_396805_397817_RPCMon2025F.root", "Muogr", "2018", "2025", "rollNames.txt")
 * [] .x muogr_v2.cxx("AnalyzeEfficiency_Aug_16Sept_2024G_RPCMon2024.root", "AnalyzeEfficiency_396805_397817_RPCMon2025F.root", "Muogr", "2024", "2025", "rollNames.txt")
 * [] .x muogr_v2.cxx("AnalyzeEfficiency_321475_325172_RPCMon_2018D.root", "AnalyzeEfficiency_Aug_16Sept_2024G_RPCMon2024.root", "Muogr", "2018", "2024", "rollNames.txt")
 */

void muogr_v2(const char *fileRef,
              const char *fileComp,
              const char *histoPath,
              const char *year1 = "2018",
              const char *year2 = "2025",
              const char *rollNamesFile = "rollNames.txt")
{
    const std::string outDir = "output/";
    const std::string sYear1 = std::string(year1);
    const std::string sYear2 = std::string(year2);
    const std::string sVsTag = sYear1 + "vs" + sYear2;

    // --- create output directory only if it does not exist ---
    if (gSystem->AccessPathName(outDir.c_str()))
        gSystem->mkdir(outDir.c_str(), kTRUE);

    // --- guard: check all input files exist ---
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
    if (gSystem->AccessPathName(rollNamesFile))
    {
        std::cerr << "ERROR: roll names file not found: " << rollNamesFile << std::endl;
        return;
    }

    // --- load roll names from text file ---
    std::set<std::string> targetChambers;
    std::ifstream ifs(rollNamesFile);
    std::string line;
    while (std::getline(ifs, line))
    {
        // skip empty lines and comment lines starting with #
        if (line.empty() || line[0] == '#')
            continue;
        targetChambers.insert(line);
    }
    ifs.close();

    if (targetChambers.empty())
    {
        std::cerr << "ERROR: no roll names loaded from " << rollNamesFile << std::endl;
        return;
    }

    std::cout << "\n  Reference    (" << sYear1 << ") : " << fileRef << std::endl;
    std::cout << "  Comparison   (" << sYear2 << ") : " << fileComp << std::endl;
    std::cout << "  HistoPath                   : " << histoPath << std::endl;
    std::cout << "  Roll names file             : " << rollNamesFile << std::endl;
    std::cout << "  Roll names loaded           : " << targetChambers.size() << "\n"
              << std::endl;

    TFile *fileR = TFile::Open(fileRef);
    if (!fileR || fileR->IsZombie())
    {
        std::cerr << "ERROR: could not open reference file: " << fileRef << std::endl;
        return;
    }
    if (!fileR->cd(histoPath))
    {
        std::cerr << "ERROR: path '" << histoPath << "' not found in: " << fileRef << std::endl;
        return;
    }
    TDirectory *dirR = gDirectory;
    TIter iterR(dirR->GetListOfKeys());
    TKey *keyR;

    TFile *fileC = TFile::Open(fileComp);
    if (!fileC || fileC->IsZombie())
    {
        std::cerr << "ERROR: could not open comparison file: " << fileComp << std::endl;
        return;
    }
    if (!fileC->cd(histoPath))
    {
        std::cerr << "ERROR: path '" << histoPath << "' not found in: " << fileComp << std::endl;
        return;
    }
    TDirectory *dirC = gDirectory;

    TCanvas *cR = new TCanvas("cR", "cR", 558, 409, 900, 600);
    TCanvas *cC = new TCanvas("cC", "cC", 558, 409, 900, 600);
    TCanvas *crelDif = new TCanvas("crelDif", "crelDif", 558, 409, 900, 600);

    std::string histTitle = "Relative assymetry Eff(" + sYear1 + ") vs Eff(" + sYear2 + ")";
    TH1F *hmyAssymetry = new TH1F("hmyAssymetry", histTitle.c_str(), 44, -1.1, 1.1);
    double myMean = 9.;
    double mySigma = 99.;
    double fractionOne = 9.;

    int myCount = 0;
    while ((keyR = (TKey *)iterR.Next()))
    {
        myCount++;

        TClass *clR = gROOT->GetClass(keyR->GetClassName());
        if (!clR || !clR->InheritsFrom("TH1"))
            continue;

        if (targetChambers.find(keyR->GetName()) == targetChambers.end())
            continue;

        TH1 *hR = (TH1 *)keyR->ReadObj();
        std::string outNameR = outDir + std::string(keyR->GetName()) + "_" + sYear1 + ".png";
        std::replace(outNameR.begin(), outNameR.end(), '-', 'M');
        std::replace(outNameR.begin(), outNameR.end(), '+', 'P');
        std::string strforCompR = std::string(keyR->GetName());
        std::replace(strforCompR.begin(), strforCompR.end(), '-', 'M');
        std::replace(strforCompR.begin(), strforCompR.end(), '+', 'P');
        std::cout << outNameR.c_str() << std::endl;
        cR->cd();
        gStyle->SetOptStat("nemriou");
        gStyle->SetOptFit(1);
        hR->Draw("colz");
        cR->SaveAs(outNameR.c_str());
        int countZeros = 0;

        std::string relRatioTitle = "(Eff(" + sYear1 + ")-Eff(" + sYear2 + "))/(Eff(" + sYear1 + ")+Eff(" + sYear2 + ")) " + std::string(keyR->GetName());
        TH1F *myRelDiff1D = new TH1F("myRelDiff1D", relRatioTitle.c_str(), 201, -2., +2.);

        TH1 *hC = (TH1 *)dirC->FindObjectAny(keyR->GetName());

        if (hC && gROOT->GetClass(hC->ClassName())->InheritsFrom("TH1"))
        {
            std::string outNameC = outDir + std::string(hC->GetName()) + "_" + sYear2 + ".png";
            std::replace(outNameC.begin(), outNameC.end(), '-', 'M');
            std::replace(outNameC.begin(), outNameC.end(), '+', 'P');

            myMean = 9.;
            mySigma = 99.;
            fractionOne = 9.;
            std::cout << outNameC.c_str() << std::endl;
            cC->cd();
            hC->Draw("COLZ");
            cC->SaveAs(outNameC.c_str());

            std::string outNameRelRatio1Dpng = outDir + strforCompR + "_" + sVsTag + "_relDiff1D.png";
            std::string outNameRelRatio1D = outDir + strforCompR + "_" + sVsTag + "_relDiff1D.C";

            crelDif->cd();
            int xmax = hR->GetXaxis()->GetNbins();
            int ymax = hR->GetYaxis()->GetNbins();
            std::cout << "\nnumber of bins = " << xmax << "*" << ymax << " = " << xmax * ymax << std::endl;
            double rel = 99.;
            double aR = -1;
            double aC = -1;

            for (int i = 1; i <= xmax; i++)
            {
                for (int j = 1; j <= ymax; j++)
                {
                    aR = hR->GetBinContent(i, j);
                    aC = hC->GetBinContent(i, j);
                    if (aR == 0 && aC == 0)
                    {
                        countZeros++;
                        myRelDiff1D->Fill(-2);
                    }
                    else
                    {
                        rel = (aR - aC) / (aR + aC);
                        myRelDiff1D->Fill(rel);
                    }
                }
            }

            gStyle->SetOptStat("eiou");
            gStyle->SetOptFit(1);
            myRelDiff1D->SetFillColor(kBlue + 1);
            myRelDiff1D->Draw();

            int findFitMin = myRelDiff1D->GetXaxis()->FindBin(-0.15);
            int findFitMax = myRelDiff1D->GetXaxis()->FindBin(0.15);

            if (myRelDiff1D->Integral(findFitMin, findFitMax) == 0)
            {
                std::cout << "at least one of the muography is empty and will not fit" << std::endl;
            }
            else
            {
                TF1 *funcG = new TF1("funcG", "gaus", -0.15, 0.15);
                funcG->SetLineColor(kRed + 1);
                funcG->SetLineWidth(3);
                myRelDiff1D->Fit("funcG", "Lre");
                crelDif->SaveAs(outNameRelRatio1D.c_str());
                crelDif->SaveAs(outNameRelRatio1Dpng.c_str());
                myMean = funcG->GetParameter(1);
                mySigma = funcG->GetParameter(2);
                std::cout << std::fixed << std::setprecision(5);
                std::cout << "myMean " << myMean << std::endl;
                std::cout << "mySigma " << mySigma << std::endl;
                delete funcG;
            }

            int findOne = myRelDiff1D->GetXaxis()->FindBin(1.);
            int findMOne = myRelDiff1D->GetXaxis()->FindBin(-1.);
            int findZero = myRelDiff1D->GetXaxis()->FindBin(0.);

            int entriesAtOne = myRelDiff1D->GetBinContent(findOne);

            double myIntegralNegative = myRelDiff1D->Integral(findMOne, findZero - 1);
            double myIntegralpositive = myRelDiff1D->Integral(findZero + 1, findOne);

            if ((myIntegralpositive + myIntegralNegative) > 0)
            {
                double myAssimetry = (myIntegralpositive - myIntegralNegative) / (myIntegralpositive + myIntegralNegative);
                hmyAssymetry->Fill(myAssimetry);
                int allEntries = myRelDiff1D->Integral();
                fractionOne = (entriesAtOne * 100.) / (allEntries * 1.);
                std::cout << std::fixed << std::setprecision(5);
                std::cout << "myAssimetry " << myAssimetry << std::endl;
                std::cout << "fraction at one " << fractionOne << std::endl;
                std::cout << std::endl;
            }
        }

        delete hR;
        delete myRelDiff1D;
    }

    fileR->Close();
    fileC->Close();

    delete cR;
    delete cC;
    delete crelDif;
}
