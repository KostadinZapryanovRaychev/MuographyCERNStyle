#ifndef RELDIFFBYWHEELDISK_H
#define RELDIFFBYWHEELDISK_H

#include <string>
#include <map>
#include "MuographyAnalysis.h" // DecorateFn

class TH1F;
class TCanvas;
class TFile;

// Analogous to EfficiencySummary's per-wheel/per-disk grid, but for the 1D
// relative-difference distribution: instead of one summary number per roll,
// this merges every roll's full (Eff(y1)-Eff(y2))/(Eff(y1)+Eff(y2))
// distribution into one aggregate histogram per wheel (barrel) and per disk
// (endcap), so you see the overall spread for a whole wheel/disk at once
// instead of roll by roll.
class RelDiffByWheelDisk
{
public:
    RelDiffByWheelDisk(const std::string &sYear1, const std::string &sYear2);
    ~RelDiffByWheelDisk();

    // Merges this roll's already-filled 1D relative-difference histogram
    // into the aggregate for its wheel (barrel) or disk (endcap), parsed
    // from its raw name (e.g. "Muography_W-2_RB1in_S07_Backward" or
    // "Muography_RE-1_R2_CH01_A"). No-op if the name doesn't parse.
    void addRoll(const std::string &rawRollName, TH1F *rollRelDiff1D);

    // Draws + saves one histogram per wheel/disk (5 + 8 = 13 total): a PNG
    // into outDir, and — if outFile is non-null — an entry into it too.
    void save(const std::string &outDir, TFile *outFile, TCanvas *c, const DecorateFn &decorate);

private:
    std::map<int, TH1F *> barrel_; // wheel (-2..2) -> aggregate histogram
    std::map<int, TH1F *> endcap_; // disk (-4..-1, 1..4) -> aggregate histogram
    std::string sYear1_, sYear2_;
};

#endif
