#ifndef EFFICIENCYSUMMARY_H
#define EFFICIENCYSUMMARY_H

#include <string>
#include <map>
#include <vector>

class TH2F;

// Builds the same per-wheel/per-disk summary grids that
// SummaryAnalyzeEfficiency*.root carries (Barrel/FiducialCutEffW<wheel>,
// Endcap/FiducialCutEffD<disk>) — same binning, same axis labels — but
// filled with our own per-roll value (e.g. the relative efficiency
// difference mean) instead of raw efficiency. The result can be handed
// to drawEfficiencyHistograms.C unmodified.
class EfficiencySummary
{
public:
    // templateFile: an existing SummaryAnalyzeEfficiency-shaped ROOT file.
    // Its histograms' binning/axis labels are cloned (content reset to 0);
    // that's the only thing it's used for.
    explicit EfficiencySummary(const std::string &templateFile);
    ~EfficiencySummary();

    // rawRollName: the roll's raw histogram key name as stored in Muogr,
    // e.g. "Muography_W-2_RB1in_S07_Backward" or
    // "Muography_RE-1_R2_CH01_A". Parses wheel/sector/station (barrel) or
    // disk/ring/chamber/segment (endcap) from it and sets that bin to
    // value. Silently does nothing if the name doesn't parse or no
    // matching bin is found (e.g. roll type not present in the template).
    void fillRoll(const std::string &rawRollName, double value);

    // Writes every grid under Barrel/ and Endcap/ into outPath.
    void save(const std::string &outPath) const;

    // True if the template loaded successfully (at least one grid found).
    bool isValid() const;

private:
    std::map<int, TH2F *> barrel_; // wheel (-2..2) -> grid
    std::map<int, TH2F *> endcap_; // disk (-4..-1, 1..4) -> grid

    void fillBarrelRoll(const std::vector<std::string> &tokens, double value);
    void fillEndcapRoll(const std::vector<std::string> &tokens, double value);
};

#endif
