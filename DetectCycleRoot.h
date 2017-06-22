//
// Created by Rohit on 21-Jun-17.
//

#ifndef CYCLEDETECTION_DETECTCYCLEROOT_H
#define CYCLEDETECTION_DETECTCYCLEROOT_H
#include <string>
#include <iostream>
#include <map>
#include <set>
typedef std::string nodeid;

typedef std::set<nodeid> nodeSet;
typedef std::map<long, nodeSet> timeGroup;
int findRootNodes(std::string input,std::string output, int window,bool timeInMsec,int cleanUpLimit);
std::set<nodeid> findCandidateWhileUpdate(std::string fromNode, std::string toNode, std::map<nodeid, timeGroup> *completeSummary, long currenttime,
                                     long window_bracket,  timeGroup::iterator *it);
#endif //CYCLEDETECTION_DETECTCYCLEROOT_H
