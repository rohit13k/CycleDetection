//
// Created by Rohit on 04-Sep-17.
//

#ifndef CYCLEDETECTION_CYCLEROOTFINDER_H
#define CYCLEDETECTION_CYCLEROOTFINDER_H
#include <string>
#include "DetectCycle.h"
int findRootNodesApprox(std::string input, std::string output, int window,int cleanUpLimit,bool reverseEdge);
int cleanup(std::map<string, std::map<long, std::set<long>>> *completeSummary, long timestamp, long window_bracket);
int getCandidatesSize(std::map<long, std::set<long>> summary, long t_s, long t_e);
#endif //CYCLEDETECTION_CYCLEROOTFINDER_H
