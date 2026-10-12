#include "CMS_lumi.h"

#include "TPad.h"
#include "TLatex.h"

// Same constants as the official CMS_lumi.C macro.
static const float cmsTextFont = 61;
static const bool writeExtraText = true;
static const float extraTextFont = 52;
static const float lumiTextSize = 0.6;
static const float lumiTextOffset = 0.2;
static const float cmsTextSize = 0.75;
static const float relPosX = 0.045;
static const float relPosY = 0.035;
static const float relExtraDY = 1.2;
static const float extraOverCmsTextSize = 0.76;

void CMS_lumi(TPad *pad, const std::string &rightText,
             int iPosX, const std::string &extraText)
{
    bool outOfFrame = (iPosX / 10 == 0);

    int alignX_ = 2;
    if (iPosX / 10 == 1)
        alignX_ = 1;
    else if (iPosX / 10 == 2)
        alignX_ = 2;
    else if (iPosX / 10 == 3)
        alignX_ = 3;
    int align_ = 10 * alignX_ + 3;

    float t = pad->GetTopMargin();
    float l = pad->GetLeftMargin();
    float r = pad->GetRightMargin();
    float b = pad->GetBottomMargin();

    pad->cd();

    TLatex latex;
    latex.SetNDC();
    latex.SetTextAngle(0);
    latex.SetTextColor(kBlack);

    float extraTextSize = extraOverCmsTextSize * cmsTextSize;

    latex.SetTextFont(42);
    latex.SetTextAlign(31);
    latex.SetTextSize(lumiTextSize * t);
    latex.DrawLatex(1 - r, 1 - t + lumiTextOffset * t, rightText.c_str());

    if (outOfFrame)
    {
        latex.SetTextFont(cmsTextFont);
        latex.SetTextAlign(11);
        latex.SetTextSize(cmsTextSize * t);
        latex.DrawLatex(l, 1 - t + lumiTextOffset * t, "CMS");
        return;
    }

    float posX_ = l + relPosX * (1 - l - r);
    if (iPosX % 10 == 2)
        posX_ = l + 0.5 * (1 - l - r);
    else if (iPosX % 10 == 3)
        posX_ = 1 - r - relPosX * (1 - l - r);

    float posY_ = 1 - t - relPosY * (1 - t - b);

    latex.SetTextFont(cmsTextFont);
    latex.SetTextSize(cmsTextSize * t);
    latex.SetTextAlign(align_);
    latex.DrawLatex(posX_, posY_, "CMS");

    if (writeExtraText)
    {
        latex.SetTextFont(extraTextFont);
        latex.SetTextAlign(align_);
        latex.SetTextSize(extraTextSize * t);
        latex.DrawLatex(posX_, posY_ - relExtraDY * cmsTextSize * t, extraText.c_str());
    }
}
