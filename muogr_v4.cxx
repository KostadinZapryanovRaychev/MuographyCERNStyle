#include "CmsHelpers.h"
#include "CmsHelpers.cxx"

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
#include "TLatex.h"
#include "TPaveStats.h"

void saveHistogramCanvas(TCanvas *c,
                         const std::string &outPathC,
                         const std::string &outPathPng)
{
    c->SaveAs(outPathC.c_str());
    c->SaveAs(outPathPng.c_str());
}

void saveHistogramRoot(TH1 *h, const std::string &outPath)
{
    TFile f(outPath.c_str(), "RECREATE");
    h->Write();
    f.Close();
}

void setStats(TH1 *h, bool enable)
{
    if (h)
        h->SetStats(enable);
}

void drawSingleHistogram(TCanvas *c,
                         TH1 *h,
                         const char *drawOpt,
                         const std::string &label,
                         const std::string &year,
                         const std::string &outPath)
{
    c->cd();

    h->SetStats(false);

    c->SetTopMargin(0.12);
    c->SetRightMargin(0.15);

    h->SetTitle("");
    h->GetYaxis()->SetTitle("Local y [cm]");

    h->Draw(drawOpt);

    TLatex extraY;
    extraY.SetNDC();
    extraY.SetTextFont(42);
    extraY.SetTextSize(0.04);
    extraY.SetTextAngle(90); // vertical text like Y axis
    extraY.DrawLatex(0.97, 0.6, "Efficiency [%]");

    drawCMSHeader(label, getEnergyLabel(year));
    // c->SetBottomMargin(0.18);

    // drawEfficiencyCaption(
    //     "Muon efficiency map:",
    //     "Comparison between 2018 and 2025 RPC efficiencies");

    c->Modified();
    c->Update();

    c->SaveAs(outPath.c_str());
}

void drawRelDiff(TCanvas *c, TH1F *h,
                 const std::string &,
                 const std::string &)
{
    c->cd();

    h->SetStats(true);
    h->SetFillColor(kBlue + 1);

    h->Draw();

    // c->SetBottomMargin(0.18);

    // drawEfficiencyCaption(
    //     "Muon efficiency map 2 :",
    //     "Comparison between 2018 and 2025 RPC efficiencies Trying to fit caption 2 TEST");

    c->Update();

    TPaveStats *st = (TPaveStats *)h->FindObject("stats");

    if (st)
    {
        st->SetX1NDC(0.70);
        st->SetX2NDC(0.90);
        st->SetY1NDC(0.55);
        st->SetY2NDC(0.75);
    }

    std::string label = "2018 vs 2025 data";
    drawCMSPreliminaryOverlay(label);

    c->Modified();
    c->Update();
}

void saveRelDiffCanvas(TCanvas *c,
                       const std::string &outPathC,
                       const std::string &outPathPng)
{
    c->SaveAs(outPathC.c_str());
    c->SaveAs(outPathPng.c_str());
}

void createCanvases(TCanvas *&cR, TCanvas *&cC, TCanvas *&crelDif)
{
    cR = new TCanvas("cR", "cR", 558, 409, 900, 600);
    cC = new TCanvas("cC", "cC", 558, 409, 900, 600);
    crelDif = new TCanvas("crelDif", "crelDif", 558, 409, 900, 600);
}

TH1F *createAsymmetryHistogram(const std::string &sYear1,
                               const std::string &sYear2)
{
    std::string title = buildAsymmetryTitle(sYear1, sYear2);
    TH1F *h = new TH1F("hmyAssymetry", title.c_str(), 100, -1., +1.);
    setStats(h, true);
    return h;
}

void fillRelDiff(TH1 *hR, TH1 *hC, TH1F *hOut, int &countZeros)
{
    int xmax = hR->GetXaxis()->GetNbins();
    int ymax = hR->GetYaxis()->GetNbins();

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

void computeAsymmetry(TH1F *relDiff,
                      TH1F *hmyAssymetry,
                      double &fractionOne)
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
        fractionOne = (entriesAtOne * 100.) / allEntries;
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
    if (!hR)
        return;

    std::string safeName = sanitizeName(extractChamberName(keyR->GetName()));

    std::string outNameR = outDir + safeName + "_" + sYear1 + ".png";
    drawSingleHistogram(cR, hR, "colz", safeName, sYear1, outNameR);

    TH1 *hC = (TH1 *)dirC->FindObjectAny(keyR->GetName());
    if (!hC || !gROOT->GetClass(hC->ClassName())->InheritsFrom("TH1"))
    {
        delete hR;
        return;
    }

    std::string outNameC = outDir + safeName + "_" + sYear2 + ".png";
    drawSingleHistogram(cC, hC, "COLZ",
                        sanitizeName(extractChamberName(hC->GetName())),
                        sYear2, outNameC);

    if (hR)
        saveHistogramRoot(hR, outDir + safeName + "_" + sYear1 + ".C");

    if (hC)
        saveHistogramRoot(hC, outDir + safeName + "_" + sYear2 + ".C");

    std::string relRatioTitle =
        "(Eff(" + sYear1 + ")-Eff(" + sYear2 + "))/(Eff(" + sYear1 + ")+Eff(" + sYear2 + ")) " + safeName;

    TH1F *myRelDiff1D = new TH1F("myRelDiff1D", relRatioTitle.c_str(), 201, -2., +2.);

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

    TDirectory *dirR = openFileAtPath(fileRef, histoPath);
    if (!dirR)
        return;
    TDirectory *dirC = openFileAtPath(fileComp, histoPath);
    if (!dirC)
        return;

    TCanvas *cR = nullptr, *cC = nullptr, *crelDif = nullptr;
    createCanvases(cR, cC, crelDif);
    TH1F *hmyAssymetry = createAsymmetryHistogram(sYear1, sYear2);

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
