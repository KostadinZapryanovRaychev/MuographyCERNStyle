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
void drawDiffMap(TCanvas *c, TH2 *hDiff);

#endif
