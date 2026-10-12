#ifndef MUOGRAPHYANALYSIS_H
#define MUOGRAPHYANALYSIS_H

#include <string>
#include <fstream>

class TH1;
class TH1F;
class TCanvas;
class TFile;
class TKey;
class TDirectory;

void setStats(TH1 *h, bool enable);

void saveCanvasToCombinedRoot(TCanvas *c, TFile *outFile, const std::string &canvasName);

void drawMuography(TCanvas *c, TH1 *h,
                   const std::string &label,
                   const std::string &year,
                   const std::string &outPath);

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
                 TCanvas *cR, TCanvas *cC,
                 TH1F *hmyAssymetry,
                 std::ofstream &outMeansTxt,
                 TFile *combinedOutFile);

#endif
