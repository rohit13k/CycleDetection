//
// Created by Rohit on 22-Jun-17.
//

#include "FileIndexer.h"
#include <iostream>
#include <map>
#include <utility>
#include <set>
#include <sstream>
#include <fstream>
#include <vector>
#include "Split.h"
#include "bloom_filter.hpp"
#include <limits>
//struct pedge;



using namespace std;
std::map<int, std::map<int, std::set<int>>> sorteddata;// map<src,map<t,set<dst>>> such that src,dst,t is an edge

/*
 * Reads an edge list file and generate an data structure(sorteddata) which is indexed in src node and time of edge.
 * reverseEdge : Determines if the edge in file needs to read in reverse order
 */
int readFile(std::string inputFile, bool reverseEdge) {

    ifstream infile(inputFile.c_str());
    string line;
    std::vector<std::string> templine;
    long timestamp;
    int src, dst;
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if(src!=dst) {
            if (reverseEdge) {
                src = stoi(templine[1]);
                dst = stoi(templine[0]);
            }
            timestamp = stol(templine[2].c_str());
            sorteddata[src][timestamp].insert(dst);
        }
    }
    return 0;
}

void updateData(int src,int dst,int timestamp){
    sorteddata[src][timestamp].insert(dst);
}
/*
 * Returns list of edges <src,x,t> such that t is between t_s and t_end
 * and if candidates list is provided x should be in candidates
 */
std::set<pedge> getFilteredData(int src, int t_s, int t_end, set<int> *candidates) {
    std::set<pedge> result;
    set<int>::iterator xit;

    if (sorteddata.count(src) > 0) {
        std::map<int, set<int>> m = sorteddata[src];
        for (std::map<int, set<int>>::iterator low = m.lower_bound(t_s - 1); low != m.end(); ++low) {
            long t = low->first;
            if (low->first > t_end) {
                break;
            }
            //std::cout << low->first << ' ' << low->second << std::endl;
            for (xit = low->second.begin(); xit != low->second.end(); ++xit) {
                int node = *xit;
                if (candidates->size() == 0) {
                    //if candidate set is empty add all neighbours
                    pedge edge1;
                    edge1.fromVertex = src;
                    edge1.toVertex = *xit;
                    edge1.time = low->first;
                    result.insert(edge1);
                } else if (candidates->count(*xit) > 0) {
                    //if candidate set is non empty add only if the neighbour belong to the candidates set
                    pedge edge1;
                    edge1.fromVertex = src;
                    edge1.toVertex = *xit;
                    edge1.time = low->first;
                    result.insert(edge1);
                }
            }
        }

    }
    return result;
}



/*
 * Returns list of edges <src,x,t> such that t is between t_s and t_end and if candidates list is provided x should be in candidates
 */
std::set<pedge> getFilteredData(int src, int t_s, int t_end, bloom_filter *candidates) {
    std::set<pedge> result;
    set<int>::iterator xit;

    if (sorteddata.count(src) > 0) {
        std::map<int, set<int>> m = sorteddata[src];
        for (std::map<int, set<int>>::iterator low = m.lower_bound(t_s - 1); low != m.end(); ++low) {
            long t = low->first;
            if (low->first > t_end) {
                break;
            }
            //std::cout << low->first << ' ' << low->second << std::endl;
            for (xit = low->second.begin(); xit != low->second.end(); ++xit) {
                int node = *xit;
                if (candidates->contains(*xit) > 0) {
                    //if candidate set is non empty add only if the neighbour belong to the candidates set
                    pedge edge1;
                    edge1.fromVertex = src;
                    edge1.toVertex = *xit;
                    edge1.time = low->first;
                    result.insert(edge1);
                }
            }
        }

    }
    return result;
}

std::set<pedge> getFilteredData(int src, int t_s, int t_end) {
    set<int> emptycandidates;
    return getFilteredData(src, t_s, t_end, &emptycandidates);
}

// All edges of type x,src,t_x
std::set<pedge> getFilteredData(int src, int t_s) {
    std::set<pedge> result;

    if (sorteddata.count(src) > 0) {
        if (sorteddata[src].count(t_s) > 0) {
            set<int> m = sorteddata[src][t_s];
            for (auto x:m) {
                pedge edge1;
                edge1.fromVertex = src;
                edge1.toVertex = x;
                edge1.time = t_s;
                result.insert(edge1);
            }
        }
    }

    return result;
}

long getMaxTime(int src, int dst, int t_uper) {
    long result = -1;
    if (sorteddata.count(src) > 0) {
        std::map<int, set<int>> m = sorteddata[src];
        for (std::map<int, set<int>>::iterator low = m.begin(); low != m.end(); ++low) {
            long t = low->first;
            if (t > t_uper) {
                break;
            } else {
                if (low->second.count(dst) > 0) {
                    if (t > result) {
                        result = t;
                    }

                }
            }
        }

    }

    return result;
}

long getMinTime(int src, int dst, int t_lower, int t_uper) {
    long result = std::numeric_limits<int>::max();
    if (sorteddata.count(src) > 0) {
        std::map<int, set<int>> m = sorteddata[src];
        for (std::map<int, set<int>>::iterator low = m.lower_bound(t_lower - 1); low != m.end(); ++low) {
            long t = low->first;
            if (t > t_uper) {
                break;
            } else {
                if (low->second.count(dst) > 0) {
                    if (t < result) {
                        result = t;
                    }

                }
            }
        }

    }

    return result;
}

