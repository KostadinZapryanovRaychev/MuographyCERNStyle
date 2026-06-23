#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <algorithm>
#include <iomanip>

#include "TROOT.h"
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TH1.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TLatex.h"
#include "TPaveText.h"

// --------------------------------------------------
// CMS STYLE (VISUAL ONLY)
// --------------------------------------------------
void setCMSStyle()
{
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);

    gStyle->SetTextFont(42);
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetTitleFont(42, "XYZ");

    gStyle->SetPadLeftMargin(0.12);
    gStyle->SetPadRightMargin(0.05);
    gStyle->SetPadBottomMargin(0.12);
}

// --------------------------------------------------
// FULL CMS HEADER (matches ROOT macro style)
// --------------------------------------------------
void drawCMSHeader(const std::string &title,
                   const std::string &label)
{
    // -------------------------------
    // TITLE (centered, compact box)
    // -------------------------------
    TPaveText *pt = new TPaveText(0.12, 0.92, 0.88, 0.97, "blNDC");
    pt->SetBorderSize(0);
    pt->SetFillColor(0);
    pt->SetFillStyle(0);
    pt->SetTextFont(42);
    pt->SetTextSize(0.045);
    pt->SetTextAlign(22); // center horizontally
    pt->AddText(title.c_str());
    pt->Draw();

    // -------------------------------
    // CMS
    // -------------------------------
    TLatex cms;
    cms.SetNDC();
    cms.SetTextFont(61);
    cms.SetTextSize(0.06);
    cms.DrawLatex(0.15, 0.83, "CMS");

    // -------------------------------
    // Preliminary
    // -------------------------------
    TLatex prelim;
    prelim.SetNDC();
    prelim.SetTextFont(52);
    prelim.SetTextSize(0.045);
    prelim.DrawLatex(0.15, 0.78, "Preliminary");

    // -------------------------------
    // Data label
    // -------------------------------
    TLatex data;
    data.SetNDC();
    data.SetTextFont(42);
    data.SetTextSize(0.045);
    data.DrawLatex(0.15, 0.73, label.c_str());

    // -------------------------------
    // Energy (top-right)
    // -------------------------------
    TLatex energy;
    energy.SetNDC();
    energy.SetTextFont(42);
    energy.SetTextSize(0.05);
    energy.SetTextAlign(31);
    energy.DrawLatex(0.95, 0.92, "(13 TeV)");
}

// --------------------------------------------------
// MAIN (UNCHANGED LOGIC)
// --------------------------------------------------
void muogr_v3(const char *fileRef,
              const char *fileComp,
              const char *histoPath,
              const char *year1 = "2018",
              const char *year2 = "2025",
              const char *rollNamesFile = "rollNames.txt")
{
    setCMSStyle();

    const std::string outDir = "output_v3/";

    const std::string sYear1 = year1;
    const std::string sYear2 = year2;
    const std::string sVsTag = sYear1 + "vs" + sYear2;

    if (gSystem->AccessPathName(outDir.c_str()))
        gSystem->mkdir(outDir.c_str(), kTRUE);

    if (gSystem->AccessPathName(fileRef) ||
        gSystem->AccessPathName(fileComp) ||
        gSystem->AccessPathName(rollNamesFile))
    {
        std::cerr << "ERROR: missing input files\n";
        return;
    }

    std::set<std::string> targetChambers;
    std::ifstream ifs(rollNamesFile);
    std::string line;

    while (std::getline(ifs, line))
    {
        if (!line.empty() && line[0] != '#')
            targetChambers.insert(line);
    }

    TFile *fileR = TFile::Open(fileRef);
    TFile *fileC = TFile::Open(fileComp);

    fileR->cd(histoPath);
    TDirectory *dirR = gDirectory;

    fileC->cd(histoPath);
    TDirectory *dirC = gDirectory;

    TIter iterR(dirR->GetListOfKeys());
    TKey *keyR;

    TCanvas *cR = new TCanvas("cR", "", 900, 700);
    TCanvas *cC = new TCanvas("cC", "", 900, 700);
    TCanvas *cRel = new TCanvas("cRel", "", 900, 700);

    TH1F *hmyAssymetry = new TH1F(
        "hmyAssymetry",
        ("Relative asymmetry " + sYear1 + " vs " + sYear2).c_str(),
        44, -1.1, 1.1);

    while ((keyR = (TKey *)iterR.Next()))
    {
        if (!gROOT->GetClass(keyR->GetClassName())->InheritsFrom("TH1"))
            continue;

        std::string name = keyR->GetName();

        if (targetChambers.find(name) == targetChambers.end())
            continue;

        TH1 *hR = (TH1 *)keyR->ReadObj();

        std::string outNameR = outDir + name + "_" + sYear1 + ".png";
        std::replace(outNameR.begin(), outNameR.end(), '-', 'M');
        std::replace(outNameR.begin(), outNameR.end(), '+', 'P');

        // ---------------- REF PLOT ----------------
        cR->cd();
        hR->SetTitle("");
        hR->Draw("COLZ");

        drawCMSHeader("RPC Muography Efficiency", sYear1 + " data");

        cR->SaveAs(outNameR.c_str());

        TH1F *myRelDiff1D = new TH1F(
            "myRelDiff1D",
            ("Rel diff " + name).c_str(),
            201, -2., 2.);

        TH1 *hC = (TH1 *)dirC->FindObjectAny(name.c_str());

        if (hC)
        {
            std::string outNameC = outDir + name + "_" + sYear2 + ".png";
            std::replace(outNameC.begin(), outNameC.end(), '-', 'M');
            std::replace(outNameC.begin(), outNameC.end(), '+', 'P');

            // ---------------- COMP PLOT ----------------
            cC->cd();
            hC->SetTitle("");
            hC->Draw("COLZ");

            drawCMSHeader("RPC Muography Efficiency", sYear2 + " data");

            cC->SaveAs(outNameC.c_str());

            int xmax = hR->GetXaxis()->GetNbins();
            int ymax = hR->GetYaxis()->GetNbins();

            double aR, aC;

            for (int i = 1; i <= xmax; i++)
            {
                for (int j = 1; j <= ymax; j++)
                {
                    aR = hR->GetBinContent(i, j);
                    aC = hC->GetBinContent(i, j);

                    if (aR == 0 && aC == 0)
                    {
                        myRelDiff1D->Fill(-2);
                    }
                    else
                    {
                        myRelDiff1D->Fill((aR - aC) / (aR + aC));
                    }
                }
            }

            // ---------------- RELATIVE PLOT ----------------
            cRel->cd();
            myRelDiff1D->SetFillColor(kBlue + 1);
            myRelDiff1D->Draw();

            drawCMSHeader("RPC Muography Efficiency",
                          sYear1 + " vs " + sYear2);

            std::string outRel = outDir + name + "_" + sVsTag + "_rel.png";
            cRel->SaveAs(outRel.c_str());

            // ---------------- ASYMMETRY (UNCHANGED) ----------------
            int findOne = myRelDiff1D->GetXaxis()->FindBin(1.);
            int findMOne = myRelDiff1D->GetXaxis()->FindBin(-1.);
            int findZero = myRelDiff1D->GetXaxis()->FindBin(0.);

            double pos = myRelDiff1D->Integral(findZero + 1, findOne);
            double neg = myRelDiff1D->Integral(findMOne, findZero - 1);

            if ((pos + neg) > 0)
            {
                double asym = (pos - neg) / (pos + neg);
                hmyAssymetry->Fill(asym);
            }
        }

        delete hR;
        delete myRelDiff1D;
    }

    fileR->Close();
    fileC->Close();
}