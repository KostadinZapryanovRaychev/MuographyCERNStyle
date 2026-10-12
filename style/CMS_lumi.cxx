#include "CMS_lumi.h"

#include "TPad.h"
#include "TLatex.h"

// Same constants as the official CMS_lumi.C macro.
static const float cmsTextFont = 61;
static const bool writeExtraText = true;
static const float extraTextFont = 52;
static const float lumiTextSize = 0.6;
static const float lumiTextOffset = 0.2;
static const float cmsTextSize = 0.85;
static const float relPosX = 0.045;
static const float relPosY = 0.035;
static const float extraOverCmsTextSize = 0.76;

void CMS_lumi(TPad *pad, const std::string &rightText,
             int iPosX, const std::string &extraText)
{
    bool outOfFrame = (iPosX / 10 == 0);

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

    float posX_ = l; // start flush with the left edge of the plot frame
    if (iPosX % 10 == 2)
        posX_ = l + 0.5 * (1 - l - r);
    else if (iPosX % 10 == 3)
        posX_ = 1 - r - relPosX * (1 - l - r);

    // Same baseline as the right-hand (roll/year) title so "CMS Preliminary"
    // sits level with it instead of lower, inside the frame.
    float posY_ = 1 - t + lumiTextOffset * t;

    // Left-aligned, bottom-anchored so "CMS" and the extra text sit side by
    // side on that shared baseline instead of stacked.
    const int leftBottomAlign = 11;

    latex.SetTextFont(cmsTextFont);
    latex.SetTextSize(cmsTextSize * t);
    latex.SetTextAlign(leftBottomAlign);
    latex.DrawLatex(posX_, posY_, "CMS");

    // TLatex::GetXsize() needs a real font driver to measure painted text,
    // which ROOT's batch mode (-b) doesn't have (it returns 0 there), so
    // the "CMS" width is approximated instead: ~0.62 * textSize per
    // character for this bold font, plus a small gap before the next word.
    const float charWidthFactor = 0.62;
    const float cmsWidth = charWidthFactor * cmsTextSize * t * 3; // "CMS" = 3 chars

    if (writeExtraText)
    {
        const float gap = 0.012;
        latex.SetTextFont(extraTextFont);
        latex.SetTextAlign(leftBottomAlign);
        latex.SetTextSize(extraTextSize * t);
        latex.DrawLatex(posX_ + cmsWidth + gap, posY_, extraText.c_str());
    }
}
