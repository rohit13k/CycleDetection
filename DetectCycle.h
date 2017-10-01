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
#include "double_llist.h"


typedef int nodeid;

typedef std::set<nodeid> nodeSet;
typedef std::map<int, nodeSet> timeGroup;

struct approxCandidates {
    std::set<pair<int, int>> neighbours_time;
    int root_node;

    int end_time;
    bloom_filter candidates_nodes;

    bool operator==(const approxCandidates &rhs) const {
        if (root_node==rhs.root_node & end_time == rhs.end_time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const approxCandidates &rhs) const {

        if (root_node==rhs.root_node) {

            return end_time < rhs.end_time;

        } else
            return root_node<rhs.root_node;
    }

};

struct exactCandidates {
    std::set<pair<int, int>> neighbours_time;
    int root_node;

    int end_time;
    set<int> candidates_nodes;

    bool operator==(const exactCandidates &rhs) const {
        if (root_node==rhs.root_node & end_time == rhs.end_time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const exactCandidates &rhs) const {

        if (root_node==rhs.root_node) {

            return end_time < rhs.end_time;

        } else
            return root_node<rhs.root_node;
    }

};
struct tpath {
    std::vector<pedge> path;
    std::set<nodeid> seen;
    nodeid rootnode;
    long t_start;

    bool operator<(const tpath &rhs) const {
        if (rhs.t_start == t_start) {
            return rhs.path < path;
        } else
            return rhs.t_start < t_start;
    }

    bool operator==(const tpath &rhs) const {
        if (rhs.rootnode==rootnode) {
            if (rhs.t_start == t_start) {
                if (path == rhs.path) {
                    std::cout << "p" << std::endl;
                    return true;
                } else {
                    return false;
                }


            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};


struct nodeSummary {
    std::map<int, std::set<nodeid>> summary;
    node *position_in_time_list=NULL;


};
int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window,
                 bool isCompressed,bool reverseEdge,bool candidates_provided);
void unblock(nodeid v, int t_v, int t_e);
void findCycle(nodeid rootNode, int t_s, std::set<nodeid> *candidates, int window_bracket);

bool findTemporalPath(nodeid src, nodeid dst, int t_s, int t_end, std::vector<std::string> *path_till_here,
                      std::set<nodeid> seen, std::set<nodeid> *candidates);

std::set<nodeid> getCandidates(std::map<int, std::set<nodeid>> summary, int t_s, int t_e);

int findRootNodes(std::string input, std::string output, int window,int cleanUpLimit,bool reverseEdge);

set<int> getAllTime(std::set<pedge> E, nodeid dst);

bool allPath(nodeid w, nodeid rootnode, int t_s, int t_e, std::vector <std::string> path_till_here,
             std::set<nodeid> candidates, std::set<int> *cycleFound);
bool allPathWithoutCandidate(nodeid w, nodeid rootnode, int t_s, int t_e, std::vector <std::string> path_till_here,
             std::set<nodeid> seen_node, std::set<int> *cycleFound);
std::set<int> DynamicDFS(nodeid rootnode,int t_s,int t_end, std::set<int> candidates, int window_bracket,bool isCompressed,bool candidates_provided);

void findAllCycleNaive(std::string inputGraph,std::string resultFile,int window,bool reverseEdge);
int cleanup(std::map<nodeid,std::map<int, std::set<nodeid>>> *completeSummary,int timestamp,int window_bracket);
int cleanupAdv(int timestamp,int window_bracket,double_llist *last_updated_time_list);

int findRootNodesAdv(std::string input, std::string output, int window,int cleanUpLimit,bool reverseEdge);
void DynamicDFSApprox(approxCandidates candidate, int window_bracket);
bool allPathApprox(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
              bloom_filter candidates);
int findAllCycleUsingBloom(std::string dataFile, set<approxCandidates> *root_candidates, std::string output,
                           int window, bool reverseEdge);
void DynamicDFSExact(exactCandidates candidate, int window_bracket);
int findAllCycleUsingSet(std::string dataFile, set<exactCandidates> *root_candidates, std::string output,
                         int window, bool reverseEdge);
bool
allPathExact(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
             set<int> candidates);
#endif //CYCLEDETECTION_DETECTCYCLEROOT_H
