# Muographies

RPC efficiency comparison between two years, per roll.

```bash
root -l -b -q 'muogr_v4.cxx("files/AnalyzeEfficiency_321475_325172_RPCMon_2018D.root", "files/AnalyzeEfficiency_396805_397817_RPCMon2025F.root", "Muogr", "2018", "2025")'
```

- Filters to rolls in `rollNames.txt` by default. Pass a non-empty 7th arg (e.g. `"all"`) to process every roll instead.
- Output in `for_conference_2026/`: a muography PNG per roll per year, a per-bin difference map PNG, `all_results_<year1>vs<year2>.root`, `relative_diff_means_<year1>vs<year2>.txt` (one line per roll, `rollName meanRelDiff`), and `relative_efficiency_summary_<year1>vs<year2>.root` — a per-wheel/per-disk grid of `|meanRelDiff|` shaped exactly like `SummaryAnalyzeEfficiency*.root`, built by `EfficiencySummary.cxx`.
- `style/` (CMS TDR style + CMS_lumi header) is kept separate from the processing logic (`MuographyAnalysis.*`, `CmsHelpers.*`, `DiffMap.*`, `EfficiencySummary.*`).

## Rendering the per-wheel/per-disk grid plots

`relative_efficiency_summary_<year1>vs<year2>.root` can be handed, unmodified, to the official
`drawEfficiencyHistograms.C` macro from `OfflineRPCEfficiency-master` (`DQM/RPCMonitorModule/macros/`)
to produce the standard `FiducialCutEffW<wheel>` grid plots and `RE<disk>_plot` wheel/disk views —
same plots, same CMS styling, but showing `|meanRelDiff|` instead of raw efficiency.

That macro was adjusted locally (not part of this repo — it's a separately downloaded copy) for this use:
- z-axis title changed to `"Relative Efficiency Difference"`.
- Color gradient flipped: green at 0 (identical efficiency, good), red/yellow at 1 (maximally different, bad) — opposite of the original efficiency convention (where high = good = green).
- Draw option changed from `"colztext"`/`"colztext89"` to `"colz"` (color only, no per-bin numbers).

```bash
root -l -b -q 'drawEfficiencyHistograms.C(false, false, 1, "for_conference_2026/relative_efficiency_summary_2018vs2025.root")'
```
(run from inside `OfflineRPCEfficiency-master/DQM/RPCMonitorModule/macros/`; `b_ch`/`b_ch2 = false` skip the optional
chamber-issue overlay, which needs a `bChambers.root` we don't have.)

Output: `FiducialCutEffW-2.png` … `FiducialCutEffW2.png` (barrel) and `RE-4_plot.png` … `RE+4_plot.png` (endcap).

## Running on lxplus

```bash
ssh <username>@lxplus.cern.ch

# ROOT via a CMSSW environment (adjust the release to one available on lxplus)
cmsrel CMSSW_14_0_0
cd CMSSW_14_0_0/src
cmsenv
cd -

# bring the project over (first time)
git clone https://github.com/KostadinZapryanovRaychev/MuographyCERNStyle.git Muographies
cd Muographies
# ... copy/scp the files/*.root inputs and OfflineRPCEfficiency-master/ separately —
# both are large/local and not tracked in this repo.

root -l -b -q 'muogr_v4.cxx("files/AnalyzeEfficiency_321475_325172_RPCMon_2018D.root", "files/AnalyzeEfficiency_396805_397817_RPCMon2025F.root", "Muogr", "2018", "2025")'
```

If you don't use CMSSW, any LCG view with ROOT works instead, e.g.:
```bash
source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-el9-gcc13-opt/setup.sh
```
(pick an LCG release available on lxplus via `ls /cvmfs/sft.cern.ch/lcg/views/`).
