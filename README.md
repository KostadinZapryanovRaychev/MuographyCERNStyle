# Muographies

RPC efficiency comparison between two years, per roll.

```bash
root -l -b -q 'muogr_v4.cxx("files/2018.root", "files/2025.root", "Muogr", "2018", "2025")'
```

- Filters to rolls in `rollNames.txt` by default. Pass a non-empty 7th arg (e.g. `"all"`) to process every roll instead.
- Output in `for_conference_2026/`: a muography PNG per roll per year, `all_results_<year1>vs<year2>.root` (same canvases + overall asymmetry histogram), and `relative_diff_means_<year1>vs<year2>.txt` (one line per roll, `rollName meanRelDiff`).
