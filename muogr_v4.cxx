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

std::string sanitizeName(const std::string &name)
{
    std::string s = name;
    std::replace(s.begin(), s.end(), '-', 'M');
    std::replace(s.begin(), s.end(), '+', 'P');
    return s;
}

// Strips the leading "Muography_" prefix so filenames contain only the chamber name.
// e.g. "Muography_W-2_RB1in_S07_Backward" → "W-2_RB1in_S07_Backward"
std::string extractChamberName(const std::string &histoName)
{
    const std::string prefix = "Muography_";
    if (histoName.substr(0, prefix.size()) == prefix)
        return histoName.substr(prefix.size());
    return histoName;
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

// =============================================================
//  DRAWING FUNCTIONS  — add / remove / edit only in this block
// =============================================================

// [1] Draws a single 2-D efficiency histogram (reference or comparison) and saves it.
void drawSingleHistogram(TCanvas *c, TH1 *h, const char *drawOpt, const std::string &outPath)
{
    c->cd();
    gStyle->SetOptStat("nemriou");
    gStyle->SetOptFit(1);
    h->Draw(drawOpt);
    c->SaveAs(outPath.c_str());
}

// [2] Draws the 1-D relative-difference histogram onto the canvas (does not save).
void drawRelDiff(TCanvas *c, TH1F *h, const std::string & /*outPathPng*/, const std::string & /*outPathC*/)
{
    c->cd();
    gStyle->SetOptStat("eiou");
    gStyle->SetOptFit(1);
    h->SetFillColor(kBlue + 1);
    h->Draw();
}

// [3] Saves the relative-difference canvas after the Gaussian fit has been drawn on it.
void saveRelDiffCanvas(TCanvas *c, const std::string &outPathC, const std::string &outPathPng)
{
    c->SaveAs(outPathC.c_str());
    c->SaveAs(outPathPng.c_str());
}

// =============================================================
//  END OF DRAWING FUNCTIONS
// =============================================================

// ─────────────────────────────────────────────
//  Computation
// ─────────────────────────────────────────────

void fillRelDiff(TH1 *hR, TH1 *hC, TH1F *hOut, int &countZeros)
{
    int xmax = hR->GetXaxis()->GetNbins();
    int ymax = hR->GetYaxis()->GetNbins();
    std::cout << "\nnumber of bins = " << xmax << "*" << ymax << " = " << xmax * ymax << std::endl;

    for (int i = 1; i <= xmax; i++)
    {
        for (int j = 1; j <= ymax; j++)
        {
            double aR = hR->GetBinContent(i, j);
            double aC = hC->GetBinContent(i, j);
            if (aR == 0 && aC == 0)
            {
                countZeros++;
                hOut->Fill(-2);
            }
            else
            {
                hOut->Fill((aR - aC) / (aR + aC));
            }
        }
    }
}

// Returns true if the fit was performed, filling mean and sigma.
bool fitRelDiff(TH1F *h, double &mean, double &sigma)
{
    int findFitMin = h->GetXaxis()->FindBin(-0.15);
    int findFitMax = h->GetXaxis()->FindBin(0.15);

    if (h->Integral(findFitMin, findFitMax) == 0)
    {
        std::cout << "at least one of the muography is empty and will not fit" << std::endl;
        return false;
    }

    TF1 *funcG = new TF1("funcG", "gaus", -0.15, 0.15);
    funcG->SetLineColor(kRed + 1);
    funcG->SetLineWidth(3);
    h->Fit("funcG", "Lre");
    mean = funcG->GetParameter(1);
    sigma = funcG->GetParameter(2);
    std::cout << std::fixed << std::setprecision(5);
    std::cout << "myMean " << mean << std::endl;
    std::cout << "mySigma " << sigma << std::endl;
    delete funcG;
    return true;
}

void computeAsymmetry(TH1F *relDiff, TH1F *hmyAssymetry, double &fractionOne)
{
    int findOne = relDiff->GetXaxis()->FindBin(1.);
    int findMOne = relDiff->GetXaxis()->FindBin(-1.);
    int findZero = relDiff->GetXaxis()->FindBin(0.);

    int entriesAtOne = relDiff->GetBinContent(findOne);
    double myIntegralNeg = relDiff->Integral(findMOne, findZero - 1);
    double myIntegralPos = relDiff->Integral(findZero + 1, findOne);

    if ((myIntegralPos + myIntegralNeg) > 0)
    {
        double myAssimetry = (myIntegralPos - myIntegralNeg) / (myIntegralPos + myIntegralNeg);
        hmyAssymetry->Fill(myAssimetry);
        int allEntries = relDiff->Integral();
        fractionOne = (entriesAtOne * 100.) / (allEntries * 1.);
        std::cout << std::fixed << std::setprecision(5);
        std::cout << "myAssimetry " << myAssimetry << std::endl;
        std::cout << "fraction at one " << fractionOne << std::endl;
        std::cout << std::endl;
    }
}

void processRoll(TKey *keyR, TDirectory *dirC,
                 const std::string &outDir,
                 const std::string &sYear1, const std::string &sYear2,
                 const std::string &sVsTag,
                 TCanvas *cR, TCanvas *cC, TCanvas *crelDif,
                 TH1F *hmyAssymetry)
{
    TClass *clR = gROOT->GetClass(keyR->GetClassName());
    if (!clR || !clR->InheritsFrom("TH1"))
        return;

    TH1 *hR = (TH1 *)keyR->ReadObj();
    std::string safeName = sanitizeName(extractChamberName(keyR->GetName()));
    std::string outNameR = outDir + safeName + "_" + sYear1 + ".png";
    std::cout << outNameR << std::endl;

    drawSingleHistogram(cR, hR, "colz", outNameR);

    std::string relRatioTitle = "(Eff(" + sYear1 + ")-Eff(" + sYear2 + "))/(Eff(" + sYear1 + ")+Eff(" + sYear2 + ")) " + std::string(keyR->GetName());
    TH1F *myRelDiff1D = new TH1F("myRelDiff1D", relRatioTitle.c_str(), 201, -2., +2.);

    TH1 *hC = (TH1 *)dirC->FindObjectAny(keyR->GetName());

    if (hC && gROOT->GetClass(hC->ClassName())->InheritsFrom("TH1"))
    {
        std::string outNameC = outDir + sanitizeName(extractChamberName(hC->GetName())) + "_" + sYear2 + ".png";
        std::cout << outNameC << std::endl;

        drawSingleHistogram(cC, hC, "COLZ", outNameC);

        std::string outPathPng = outDir + safeName + "_" + sVsTag + "_relDiff1D.png";
        std::string outPathC = outDir + safeName + "_" + sVsTag + "_relDiff1D.C";

        int countZeros = 0;
        fillRelDiff(hR, hC, myRelDiff1D, countZeros);

        drawRelDiff(crelDif, myRelDiff1D, outPathPng, outPathC);

        double myMean = 9., mySigma = 99.;
        if (fitRelDiff(myRelDiff1D, myMean, mySigma))
            saveRelDiffCanvas(crelDif, outPathC, outPathPng);

        double fractionOne = 9.;
        computeAsymmetry(myRelDiff1D, hmyAssymetry, fractionOne);
    }

    delete hR;
    delete myRelDiff1D;
}

void muogr_v4(const char *fileRef,
              const char *fileComp,
              const char *histoPath,
              const char *year1 = "2018",
              const char *year2 = "2025",
              const char *rollNamesFile = "rollNames.txt")
{
    const std::string outDir = "output_cms_style/";
    const std::string sYear1 = std::string(year1);
    const std::string sYear2 = std::string(year2);
    const std::string sVsTag = sYear1 + "vs" + sYear2;

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
    if (gSystem->AccessPathName(rollNamesFile))
    {
        std::cerr << "ERROR: roll names file not found: " << rollNamesFile << std::endl;
        return;
    }

    std::set<std::string> targetChambers = loadRollNames(rollNamesFile);
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

    TDirectory *dirR = openFileAtPath(fileRef, histoPath);
    if (!dirR)
        return;
    TDirectory *dirC = openFileAtPath(fileComp, histoPath);
    if (!dirC)
        return;

    TCanvas *cR = new TCanvas("cR", "cR", 558, 409, 900, 600);
    TCanvas *cC = new TCanvas("cC", "cC", 558, 409, 900, 600);
    TCanvas *crelDif = new TCanvas("crelDif", "crelDif", 558, 409, 900, 600);

    std::string histTitle = "Relative assymetry Eff(" + sYear1 + ") vs Eff(" + sYear2 + ")";
    TH1F *hmyAssymetry = new TH1F("hmyAssymetry", histTitle.c_str(), 44, -1.1, 1.1);

    TIter iterR(dirR->GetListOfKeys());
    TKey *keyR;
    int myCount = 0;
    while ((keyR = (TKey *)iterR.Next()))
    {
        myCount++;
        if (targetChambers.find(keyR->GetName()) == targetChambers.end())
            continue;

        processRoll(keyR, dirC, outDir, sYear1, sYear2, sVsTag,
                    cR, cC, crelDif, hmyAssymetry);
    }

    dirR->GetFile()->Close();
    dirC->GetFile()->Close();

    delete cR;
    delete cC;
    delete crelDif;
}
