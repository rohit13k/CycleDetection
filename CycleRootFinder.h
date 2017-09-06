//
// Created by Rohit on 04-Sep-17.
//

#ifndef CYCLEDETECTION_CYCLEROOTFINDER_H
#define CYCLEDETECTION_CYCLEROOTFINDER_H

#include <string>
#include "DetectCycle.h"

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

int findRootNodesApprox(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);

int cleanup(std::map<string, map<string,long>> *completeSummary, long timestamp, long window_bracket);

set<string> getCandidates(std::map<string,long> summary, long t_s, long t_e);

#endif //CYCLEDETECTION_CYCLEROOTFINDER_H
