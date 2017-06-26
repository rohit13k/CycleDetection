//
// Created by Rohit on 21-Jun-17.
//

#ifndef CYCLEDETECTION_DETECTCYCLEROOT_H
#define CYCLEDETECTION_DETECTCYCLEROOT_H

#include <string>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "FileIndexer.h"


typedef std::string nodeid;

typedef std::set<nodeid> nodeSet;
typedef std::map<long, nodeSet> timeGroup;

int findRootNodes(std::string input, std::string output, int window, bool timeInMsec, int cleanUpLimit);

int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window, bool timeInMsec,
                 bool usingGlobalBlock);

void findCycle(std::string rootNode, long t_s, std::set<std::string> *candidates, long window_bracket);

bool findTemporalPath(std::string src, std::string dst, long t_s, long t_end, std::vector<std::string> *path_till_here,
                      std::set<std::string> seen, std::set<std::string> *candidates);

std::set<nodeid> getCandidates(std::set<std::pair<nodeid, long>> summary, long t_s, long t_e);

int findRootNodesNew(std::string input, std::string output, int window, bool timeInMsec);

std::set<long> getAllTime(std::set<pedge> E, nodeid dst);

bool allPath(nodeid w, nodeid rootnode, long t_s, long t_e, std::vector <std::string> path_till_here,
             std::set<std::string> candidates);
void DynamicDFS(nodeid rootnode,long t_s, std::set<std::string> candidates, long window_bracket);

#endif //CYCLEDETECTION_DETECTCYCLEROOT_H