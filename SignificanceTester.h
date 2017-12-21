//
// Created by Rohit on 21-Dec-17.
//

#ifndef CYCLEDETECTION_SIGNIFICANCETESTER_H
#define CYCLEDETECTION_SIGNIFICANCETESTER_H

#include <string>

void prepareData(std::string dataFile, std::string pathFile, bool reverseEdge);
void getSignificantCycle(int window,std::string output);
int binomialCoeff(int n, int k);

#endif //CYCLEDETECTION_SIGNIFICANCETESTER_H
