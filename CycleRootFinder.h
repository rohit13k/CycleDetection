//
// Created by Rohit on 04-Sep-17.
//

#ifndef CYCLEDETECTION_CYCLEROOTFINDER_H
#define CYCLEDETECTION_CYCLEROOTFINDER_H

#include <string>
#include "DetectCycle.h"
#include "bloom_filter.hpp"
struct tnode {
    std::string vertex;
    long time;

    bool operator==(const tnode &rhs) const {
        if ((vertex.compare(rhs.vertex) == 0&time==rhs.time)) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const tnode &rhs) const {

        if (time == rhs.time) {

            return vertex < rhs.vertex;

        } else
            return time < rhs.time;
    }

};

int findRootNodesNew(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);

int cleanup(std::map<int, map<int,int>> *completeSummary, int timestamp, int window_bracket);

set<int> getCandidates(std::map<int,int> *summary, int t_s, int t_e);

int cleanup(map<int, bloom_filter> *completeSummary,map<int,int> *node_update_time, int timestamp, int window_bracket);
int findRootNodesApprox(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);

#endif //CYCLEDETECTION_CYCLEROOTFINDER_H
