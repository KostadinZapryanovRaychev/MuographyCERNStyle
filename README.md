# Muographies

RPC efficiency comparison between two years, per roll.

```bash
root -l -b -q 'muogr_v4.cxx("files/AnalyzeEfficiency_321475_325172_RPCMon_2018D.root", "files/AnalyzeEfficiency_396805_397817_RPCMon2025F.root", "Muogr", "2018", "2025")'
```

- Filters to rolls in `rollNames.txt` by default. Pass a non-empty 7th arg (e.g. `"all"`) to process every roll instead.

relative difference should be printed in the way drawEfficiencyHistograms print them
