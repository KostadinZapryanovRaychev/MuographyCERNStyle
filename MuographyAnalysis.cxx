#include "MuographyAnalysis.h"
#include "CmsHelpers.h"

#include <iostream>
#include <iomanip>

#include "TROOT.h"
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TH1.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLatex.h"

void setStats(TH1 *h, bool enable)
{
    if (h)
        h->SetStats(enable);
}

void saveCanvasToCombinedRoot(TCanvas *c, TFile *outFile, const std::string &canvasName)
{
    if (!c || !outFile || !outFile->IsOpen())
        return;

    TDirectory *prevDir = gDirectory;

    outFile->cd();
    TCanvas *cClone = (TCanvas *)c->Clone(canvasName.c_str());
    cClone->Write();
    delete cClone;

    if (prevDir)
        prevDir->cd();
}

void drawMuography(TCanvas *c, TH1 *h,
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

    h->Draw("COLZ");

    TLatex extraY;
    extraY.SetNDC();
    extraY.SetTextFont(42);
    extraY.SetTextSize(0.04);
    extraY.SetTextAngle(90);
    extraY.DrawLatex(0.97, 0.6, "Efficiency [%]");

    drawCMSHeader(label, getEnergyLabel(year));

    c->Modified();
    c->Update();

    c->SaveAs(outPath.c_str());
}

TH1F *createAsymmetryHistogram(const std::string &sYear1,
                               const std::string &sYear2)
{
    std::string title = buildAsymmetryTitle(sYear1, sYear2);
    TH1F *h = new TH1F("hmyAssymetry", title.c_str(), 100, -1., +1.);
    h->SetDirectory(nullptr);
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

    h->Fit(funcG, "LRE0");

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
                 TCanvas *cR, TCanvas *cC,
                 TH1F *hmyAssymetry,
                 std::ofstream &outMeansTxt,
                 TFile *combinedOutFile)
{
    TClass *clR = gROOT->GetClass(keyR->GetClassName());
    if (!clR || !clR->InheritsFrom("TH1"))
        return;

    TH1 *hR = (TH1 *)keyR->ReadObj();
    if (!hR)
        return;

    std::string safeName = sanitizeName(extractChamberName(keyR->GetName()));

    TH1 *hC = (TH1 *)dirC->FindObjectAny(keyR->GetName());
    if (!hC || !gROOT->GetClass(hC->ClassName())->InheritsFrom("TH1"))
    {
        delete hR;
        return;
    }

    drawMuography(cR, hR, safeName, sYear1, outDir + safeName + "_" + sYear1 + ".png");
    saveCanvasToCombinedRoot(cR, combinedOutFile, safeName + "_" + sYear1);

    drawMuography(cC, hC, sanitizeName(extractChamberName(hC->GetName())), sYear2,
                 outDir + safeName + "_" + sYear2 + ".png");
    saveCanvasToCombinedRoot(cC, combinedOutFile, safeName + "_" + sYear2);

    std::string relRatioTitle =
        "(Eff(" + sYear1 + ")-Eff(" + sYear2 + "))/(Eff(" + sYear1 + ")+Eff(" + sYear2 + ")) " + safeName;

    TH1F *myRelDiff1D = new TH1F("myRelDiff1D", relRatioTitle.c_str(), 201, -2., +2.);
    myRelDiff1D->SetDirectory(nullptr);

    int countZeros = 0;
    fillRelDiff(hR, hC, myRelDiff1D, countZeros);

    double myMean = 9., mySigma = 99.;
    if (fitRelDiff(myRelDiff1D, myMean, mySigma))
    {
        outMeansTxt << safeName << " " << std::fixed << std::setprecision(5)
                    << myMean << std::endl;
    }

    double fractionOne = 9.;
    computeAsymmetry(myRelDiff1D, hmyAssymetry, fractionOne);

    delete hR;
    delete myRelDiff1D;
}
