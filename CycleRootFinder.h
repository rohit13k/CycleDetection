//
// Created by Rohit on 04-Sep-17.
//

#ifndef CYCLEDETECTION_CYCLEROOTFINDER_H
#define CYCLEDETECTION_CYCLEROOTFINDER_H

#include <string>
#include <fstream>
#include "DetectCycle.h"
#include "bloom_filter.hpp"

struct tnode {
    std::string vertex;
    long time;

    bool operator==(const tnode &rhs) const {
        if ((vertex.compare(rhs.vertex) == 0 & time == rhs.time)) {
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



struct cycle_time {
    int start_time;
    int end_time;

    bool operator==(const cycle_time &rhs) const {
        if ((start_time == rhs.start_time & end_time == rhs.end_time)) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const cycle_time &rhs) const {

        if (start_time == rhs.start_time) {

            return end_time < rhs.end_time;

        } else
            return start_time < rhs.start_time;
    }
};
int findRootNodesNew(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);

set<int> getCandidatesNew(map<int, int> *summary, int t_s, int t_e);

int cleanupNew(std::map<int, map<int, int>> *completeSummary, int timestamp, int window_bracket, bool forward);


set<int> getCandidates(std::map<int, int> *summary, int t_s, int t_e);

vector<string>
updateSummaries(map<int, map<int, set<int>>> *completeSummary, int timestamp,
                int window_bracket, int src, int dst);

int
cleanup(map<int, bloom_filter> *completeSummary, map<int, int> *node_update_time, int timestamp, int window_bracket);

int findRootNodesApprox(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);

set<approxCandidates> findRootNodesApproxBothDirection(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);

int findCandidateFromApprox(std::string input, string root_file, std::string output, int window, int cleanUpLimit,
                            bool reverseEdge);

pair<int, pair<int, int>>
updateSummary(int src, int dst, int timestamp, int window_bracket, map<int, bloom_filter> *summary,
              bloom_parameters parameters, map<int, int> *update_time);

void mergeSummaries(map<int, bloom_filter> *summary, approxCandidates *ac, int start_time);
void mergeSummaries(map<int, set<int>> *summary, exactCandidates *ac, int start_time);
set<approxCandidates>
compressRootCandidates(map<int, map<cycle_time, map<int, bloom_filter>>> *root_candidates,
                       int window_bracket);

pair<int,pair<int, int>>
updateSummaryExact(int src, int dst, int timestamp, int window_bracket, map<int, set<int>> *summary,
                   map<int, int> *update_time);

set<approxCandidatesNew>
findRootNodesApproxBothDirectionNew(std::string input, std::string output, int window, int cleanUpLimit,
                                    bool reverseEdge);
set<exactCandidates>
compressRootCandidates(map<int, map<cycle_time, map<int, set<int>>>> *root_candidates,
                       int window_bracket);
set<exactCandidates>
findRootNodesExactBothDirection(std::string input, std::string output, int window, int cleanUpLimit,
                                bool reverseEdge);
void print(map<int, map<cycle_time, map<int, set<int>>>> root_candidate_exact);
void print(set<exactCandidates> final_roots);
int cleanup(map<int, set<int>> *completeSummary, map<int, int> *node_update_time, int timestamp, int window_bracket);
string combineSeeds(std::string root_file, int window);

set<approxCandidatesNew>
compressRootCandidatesNew(map<int, map<cycle_time, map<int, bloom_filter>>> *root_candidates,
                          int window_bracket);

#endif //CYCLEDETECTION_CYCLEROOTFINDER_H
