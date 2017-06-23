//
// Created by Rohit on 22-Jun-17.
//

#ifndef CYCLEDETECTION_FILEINDEXER_H
#define CYCLEDETECTION_FILEINDEXER_H
#include <set>
#include <string>
struct pedge {
    std::string fromVertex;
    std::string toVertex;
    long time;

};
inline bool operator<(const pedge &lhs,const pedge &rhs) {
    return lhs.time < rhs.time;
}

int readFile(std::string inputFile);

std::set<pedge> getFilteredData(std::string src, long t_s, long t_end, std::set<std::string> *candidates);

std::set<pedge> getFilteredData(std::string src, long t_s);

#endif //CYCLEDETECTION_FILEINDEXER_H
