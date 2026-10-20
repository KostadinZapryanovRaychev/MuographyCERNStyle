#ifndef MUOGRAPHYANALYSIS_H
#define MUOGRAPHYANALYSIS_H

#include <string>
#include <fstream>
#include <functional>

class TH1;
class TH1F;
class TCanvas;
class TFile;
class TKey;
class TDirectory;
class EfficiencySummary;
class RelDiffByWheelDisk;

// Applies whatever visual styling/header the caller wants to a canvas that
// has already been drawn (e.g. CMS_lumi). Injected from outside so this
// file has no dependency on any particular styling scheme.
using DecorateFn = std::function<void(TCanvas *, const std::string &rightText)>;

void setStats(TH1 *h, bool enable);

void saveCanvasToCombinedRoot(TCanvas *c, TFile *outFile, const std::string &canvasName);

// Draws the raw 2D efficiency map (axis titles only) — no styling/header applied.
void drawMuography(TCanvas *c, TH1 *h);

// Draws the raw 1D relative-difference distribution (blue fill + stats
// box) — no styling/header applied.
void drawRelDiff1D(TCanvas *c, TH1F *h);

TH1F *createAsymmetryHistogram(const std::string &sYear1,
                               const std::string &sYear2);

void fillRelDiff(TH1 *hR, TH1 *hC, TH1F *hOut, int &countZeros);

bool fitRelDiff(TH1F *h, double &mean, double &sigma);

void computeAsymmetry(TH1F *relDiff,
                      TH1F *hmyAssymetry,
                      double &fractionOne);

void processRoll(TKey *keyR, TDirectory *dirC,
                 const std::string &outDir,
                 const std::string &sYear1, const std::string &sYear2,
                 TCanvas *cR, TCanvas *cC, TCanvas *cDiff, TCanvas *cRelDiff1D,
                 TH1F *hmyAssymetry,
                 std::ofstream &outMeansTxt,
                 TFile *combinedOutFile,
                 const DecorateFn &decorate,
                 EfficiencySummary *effSummary = nullptr,
                 RelDiffByWheelDisk *relDiffAgg = nullptr);

#endif
