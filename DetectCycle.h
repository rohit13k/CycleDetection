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
#include <string.h>
#include "FileIndexer.h"
#include "double_llist.h"


typedef int nodeid;

typedef std::set<nodeid> nodeSet;
typedef std::map<int, nodeSet> timeGroup;

struct approxCandidatesNew {
    std::map<int, bloom_filter> neighbours_candidates;
    int root_node;
    int start_time;
    int end_time;


    bool operator==(const approxCandidatesNew &rhs) const {
        if (root_node == rhs.root_node & end_time == rhs.end_time & start_time == rhs.start_time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const approxCandidatesNew &rhs) const {

        if (root_node == rhs.root_node) {
            if (start_time == rhs.start_time) {
                return end_time < rhs.end_time;
            } else {

                start_time<rhs.start_time;
            }

        } else
            return root_node < rhs.root_node;
    }

};

struct timeBundle {
    set<int> times;

    bool operator==(const timeBundle &rhs) const {
        return times == rhs.times;

    }

    bool operator<(const timeBundle &rhs) const {
        if (times.size() == rhs.times.size()) {
            return *times.end() < *rhs.times.end();
        } else {
            return times.size() > rhs.times.size();
        }
    }

    string string_format() {
        string result = "(";
        int count = 0;
        for (int x:times) {
            if (count == 0) {
                count++;
                result = result + to_string(x);
            } else {
                result = result + ":" + to_string(x);
            }
        }
        result = result + ")";
        return result;
    }

    int size() {
        return times.size();
    }

    int getMinTime() {
        if (size() > 0) {
            return *times.begin();
        } else {
            return -1;
        }
    }

    int getMaxTime() {
        if (size() > 0) {

            return *times.rbegin();
        } else {
            return -1;
        }
    }
};

struct edgeBundle {
    int from_node;
    int to_node;
    timeBundle time;

    bool operator==(const edgeBundle &rhs) const {
        return from_node == rhs.from_node & to_node == rhs.to_node & time == rhs.time;
    }

    bool operator<(const edgeBundle &rhs) const {

        if (from_node == rhs.from_node) {

            if (to_node == rhs.to_node) {
                return time < rhs.time;
            } else {
                return to_node < rhs.to_node;
            }

        } else
            return from_node < rhs.from_node;
    }

    string printEdgeBundle() {
        return to_string(from_node) + "," + to_string(to_node) + "," + time.string_format();
    }

    string edgeSignature() {
        return to_string(from_node) + "," + to_string(to_node);
    }
};

struct pathBundle {
    vector<edgeBundle> path;

    int getRootNode() {
        if (path.size() > 0) {
            return path[0].from_node;
        } else {
            return -1;
        }
    }

    edgeBundle getLastEdge() {
        if (path.size() == 0) {
            cout << "error" << endl;
        }
        return path[path.size() - 1];
    }

    string printPath() {
        string result = "Path Length," + to_string(path.size()) + ",";
        for (edgeBundle eb:path) {
            result = result + eb.printEdgeBundle() + ",";
        }
        return result;
    }

    string pathSignature() const {
        string result = "";
        for (edgeBundle eb:path) {
            result = result + eb.edgeSignature() + ",";
        }
        return result;
    }

    bool operator<(const pathBundle &rhs) const {

        if (path.size() == rhs.path.size() & path.size() > 0) {
            if (path[0].from_node == rhs.path[0].from_node) {
                if (path[0].to_node == rhs.path[0].to_node) {
                    string lhs_sig = pathSignature();
                    string rhs_sig = rhs.pathSignature();

                    if (lhs_sig.compare(rhs_sig) != 0) {
                        return lhs_sig < rhs_sig;
                    } else {
                        timeBundle rhs_t = rhs.path[0].time;
                        timeBundle lhs_t = path[0].time;
                        if (lhs_t.getMinTime() == rhs_t.getMinTime()) {

                            return lhs_t.size() > rhs_t.size();
                        } else {

                            return lhs_t.getMinTime() < rhs_t.getMinTime();
                        }
                    }
                } else {
                    return path[0].to_node < rhs.path[0].to_node;
                }
            } else {
                return path[0].from_node < rhs.path[0].from_node;
            }

        } else {
            return path.size() < rhs.path.size();
        }

    }

};


struct seed {
    int start_time;
    int end_time;
    set<int> candidates;

    bool operator<(const seed &rhs) const {
        if (start_time == rhs.start_time) {
            if (end_time == rhs.end_time) {
                return candidates < rhs.candidates;
            } else {
                return end_time < rhs.end_time;
            }
        } else {
            return start_time < rhs.start_time;
        }
    }

    string pringString() {
        string result = to_string(start_time) + "," + to_string(end_time);
        for (auto x:candidates) {
            result = result + "," + to_string(x);
        }
        return result;
    }
};

struct edge {
    int node;
    int time;

    bool operator==(const edge &rhs) const {
        if ((time == rhs.time & node == rhs.node)) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const edge &rhs) const {

        if (node == rhs.node) {

            return time < rhs.time;

        } else
            return node < rhs.node;
    }
};

struct exactCandidates {
    std::set<pair<int, int>> neighbours_time; //set of nodeid,start_time
    int root_node;

    int end_time;
    set<int> candidates_nodes;

    bool operator==(const exactCandidates &rhs) const {
        if (root_node == rhs.root_node & end_time == rhs.end_time) {
            return true;
        } else {
            false;
        }
    }

    bool operator<(const exactCandidates &rhs) const {

        if (root_node == rhs.root_node) {

            return end_time < rhs.end_time;

        } else
            return root_node < rhs.root_node;
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
        if (rhs.rootnode == rootnode) {
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
    node *position_in_time_list = NULL;


};
struct endNode {
   int node_id;
    int end_time;
    bloom_filter candidates;
    bool operator<(const endNode &rhs) const {
        if (rhs.node_id == node_id) {
            return end_time<rhs.end_time;
        } else
            return node_id < rhs.end_time;
    }

    bool operator==(const endNode &rhs) const {
        if (node_id==rhs.node_id&end_time==rhs.end_time) {
           return true;
        } else {
            return false;
        }
    }

};
struct endNodeNew {
    unsigned int node_id;
    unsigned int end_time;

    bool operator<(const endNodeNew &rhs) const {
        if (rhs.node_id == node_id) {
            return end_time<rhs.end_time;
        } else
            return node_id < rhs.end_time;
    }

    bool operator==(const endNodeNew &rhs) const {
        if (node_id==rhs.node_id&end_time==rhs.end_time) {
            return true;
        } else {
            return false;
        }
    }

};

struct monitor{
    int cycles[50]={0};
    int edge_count=0;
    int candidate_count=0;
    void clear(){
        memset(cycles, 0, sizeof(cycles));
        edge_count=0;
        candidate_count=0;
    }
};

int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window,
                 bool isCompressed, bool reverseEdge, bool candidates_provided, bool use_bundle);

void unblock(nodeid v, int t_v, int t_e);

void findCycle(nodeid rootNode, int t_s, std::set<nodeid> *candidates, int window_bracket);

bool findTemporalPath(nodeid src, nodeid dst, int t_s, int t_end, std::vector<std::string> *path_till_here,
                      std::set<nodeid> seen, std::set<nodeid> *candidates);

std::set<nodeid> getCandidates(std::map<int, std::set<nodeid>> summary, int t_s, int t_e);

int findRootNodes(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);

set<int> getAllTime(std::set<pedge> E, nodeid dst);

bool allPath(nodeid w, nodeid rootnode, int t_s, int t_e, std::vector<std::string> path_till_here,
             std::set<nodeid> candidates, vector<int> *cycleLengthArray,monitor *m);

bool allPathWithoutCandidate(nodeid w, nodeid rootnode, int t_s, int t_e, std::vector<std::string> path_till_here,
                             std::set<nodeid> seen_node, vector<int> *cycleLengthArray);

string DynamicDFS(nodeid rootnode, int t_s, int t_end, std::set<int> candidates, int window_bracket,
                bool isCompressed, bool candidates_provided, bool use_bundle, vector<int> *cycleLengthArray);

void findAllCycleNaive(std::string inputGraph, std::string resultFile, int window, bool reverseEdge);

int cleanup(std::map<nodeid, std::map<int, std::set<nodeid>>> *completeSummary, int timestamp, int window_bracket);

int cleanupAdv(int timestamp, int window_bracket, double_llist *last_updated_time_list);

int findRootNodesAdv(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge);



bool allPathApprox(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
                   bloom_filter candidates);



void DynamicDFSExact(exactCandidates candidate, int window_bracket, bool use_bundle, vector<int> *cycleLengthArray);

int findAllCycleUsingSet(std::string dataFile, set<exactCandidates> *root_candidates, std::string output,
                         int window, bool reverseEdge, bool use_bundle);

bool allPathExact(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
                  set<int> candidates);


int allPathBundle(pathBundle path_bundle_till_here, int t_e, std::set<int> candidates, vector<int> *cycleLengthArray,monitor *m);

int
allPathBundleApprox(pathBundle path_bundle_till_here, int t_e, bloom_filter candidates, vector<int> *cycleLengthArray);

pathBundle expandPathBundle(pathBundle current_path, edgeBundle new_edge);

int pathCount(pathBundle pb);

void testCountPath();

// To compare two points
class myComparator {
public:
    int operator()(const pair<int, int> &p1, const pair<int, int> &p2) {
        return p1.first > p2.first;
    }
};

bool is_overlapping(pathBundle *pathBundle1, pathBundle *pathBundle2);

int findAllCycleUsingBloom(std::string dataFile, set<approxCandidatesNew> *root_candidates, std::string output,
                              int window, bool reverseEdge, bool use_bundle);

void
DynamicDFSApprox(approxCandidatesNew candidate, int window_bracket, bool use_bundle, vector<int> *cycleLengthArray);

#endif //CYCLEDETECTION_DETECTCYCLEROOT_H
