//
// Created by Rohit on 22-Jun-17.
//

#ifndef CYCLEDETECTION_FILEINDEXER_H
#define CYCLEDETECTION_FILEINDEXER_H

#include <set>
#include <string>
#include <map>
#include <iostream>
#include "bloom_filter.hpp"

struct pedge {
    int fromVertex;
   int toVertex;
    int time;

    bool operator==(const pedge &rhs) const {
        if ((fromVertex==rhs.fromVertex) && (toVertex==rhs.toVertex) && time == rhs.time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const pedge &rhs) const {

        if (time == rhs.time) {
            if (fromVertex==rhs.fromVertex) {
                return toVertex < rhs.toVertex;
            } else {
                return fromVertex < rhs.fromVertex;
            }
        } else
            return time < rhs.time;
    }

};


int readFile(std::string inputFile,bool reverseEdge);

std::set<pedge> getFilteredData(int src, int t_s, int t_end, std::set<int> *candidates);
std::set<pedge> getFilteredData(int src, int t_s, int t_end);
std::set<pedge> getFilteredData(int src, int t_s);

long getMaxTime(int src,int dst, int t_uper);
std::set<pedge> getFilteredData(int src, int t_s, int t_end, bloom_filter *candidates);
long getMinTime(int src, int dst, int t_lower, int t_uper);

#endif //CYCLEDETECTION_FILEINDEXER_H
