#ifndef CMSHELPERS_H
#define CMSHELPERS_H

#include <string>
#include <set>

std::string getEnergyLabel(const std::string &year);

std::string sanitizeName(const std::string &name);

std::string extractChamberName(const std::string &histoName);

std::set<std::string> loadRollNames(const char *rollNamesFile);

TDirectory *openFileAtPath(const char *filePath, const char *histoPath);

std::string buildAsymmetryTitle(const std::string &sYear1,
                                const std::string &sYear2);

#endif