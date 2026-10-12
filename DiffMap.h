#ifndef DIFFMAP_H
#define DIFFMAP_H

#include <string>

class TH1;
class TH2;
class TCanvas;

// Builds a new 2D histogram with the same binning as hR/hC, where each bin
// holds the relative efficiency difference (effR - effC) / (effR + effC)
// at that (x,y) spot — i.e. the "2018 map minus 2025 map" picture, bin by
// bin, as opposed to the 1D distribution used for the fit/mean.
TH2 *buildDiffMap(TH1 *hR, TH1 *hC, const std::string &name, const std::string &title);

// Draws the raw 2D difference map (axis titles only) — no styling/header applied.
// Switches the active ROOT color palette to a diverging one (blue-white-red,
// centered at 0) for the duration of the draw, since this is a signed ratio
// and the default sequential efficiency-map palette reads poorly for it.
void drawDiffMap(TCanvas *c, TH2 *hDiff);

// Restores the sequential palette used for the efficiency maps. Call after
// drawDiffMap's canvas has been fully saved (SaveAs/Write), since ROOT only
// resolves the palette at paint time.
void restoreEfficiencyPalette();

#endif
