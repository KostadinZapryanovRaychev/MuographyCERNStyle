#include "DiffMap.h"

#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"

TH2 *buildDiffMap(TH1 *hR, TH1 *hC, const std::string &name, const std::string &title)
{
    TH2 *hDiff = (TH2 *)hR->Clone(name.c_str());
    hDiff->Reset();
    hDiff->SetTitle(title.c_str());
    hDiff->SetDirectory(nullptr);

    int xmax = hR->GetXaxis()->GetNbins();
    int ymax = hR->GetYaxis()->GetNbins();

    for (int i = 1; i <= xmax; i++)
    {
        for (int j = 1; j <= ymax; j++)
        {
            double aR = hR->GetBinContent(i, j);
            double aC = hC->GetBinContent(i, j);

            double diff = 0.;
            if (aR != 0 || aC != 0)
                diff = (aR - aC) / (aR + aC);

            hDiff->SetBinContent(i, j, diff);
        }
    }

    return hDiff;
}

void drawDiffMap(TCanvas *c, TH2 *hDiff)
{
    c->cd();
    c->SetRightMargin(0.14); // room for the COLZ palette, not a style override

    hDiff->SetTitle("");
    hDiff->GetYaxis()->SetTitle("Local y [cm]");
    hDiff->GetZaxis()->SetTitle("(Eff_{1} - Eff_{2}) / (Eff_{1} + Eff_{2})");
    hDiff->SetMinimum(-1.);
    hDiff->SetMaximum(1.);

    hDiff->Draw("COLZ");
}
