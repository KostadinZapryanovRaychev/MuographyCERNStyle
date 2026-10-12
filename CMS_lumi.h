#ifndef CMS_LUMI_H
#define CMS_LUMI_H

#include <string>
class TPad;

// Faithful reproduction of the public CMS "CMS_lumi.C" macro's label logic
// (https://ghm.web.cern.ch/ghm/plots/), adapted to take the right-hand
// text directly instead of selecting it from a hardcoded iPeriod table.
// iPosX follows the original convention: tens digit = horizontal position
// (0 = out of frame, 1 = left, 2 = center, 3 = right), units digit = align.
void CMS_lumi(TPad *pad, const std::string &rightText,
             int iPosX = 11, const std::string &extraText = "Preliminary");

#endif
