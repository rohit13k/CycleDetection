//
// Created by Rohit on 22-Jun-17.
//

#ifndef CYCLEDETECTION_FILEINDEXER_H
#define CYCLEDETECTION_FILEINDEXER_H

#include <set>
#include <string>
#include <map>
#include <iostream>

struct pedge {
    std::string fromVertex;
    std::string toVertex;
    long time;

    bool operator==(const pedge &rhs) const {
        if ((fromVertex.compare(rhs.fromVertex) == 0) && (toVertex.compare(rhs.toVertex) == 0) && time == rhs.time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const pedge &rhs) const {

        if (time == rhs.time) {
            if (fromVertex.compare(rhs.fromVertex) == 0) {
                return toVertex < rhs.toVertex;
            } else {
                return fromVertex < rhs.fromVertex;
            }
        } else
            return time < rhs.time;
    }

};


int readFile(std::string inputFile);

std::set<pedge> getFilteredData(std::string src, long t_s, long t_end, std::set<std::string> *candidates);

std::set<pedge> getFilteredData(std::string src, long t_s);

long getMaxTime(std::string src, std::string dst, long t_uper);

long getMinTime(std::string src, std::string dst, long t_lower, long t_uper);

#endif //CYCLEDETECTION_FILEINDEXER_H
