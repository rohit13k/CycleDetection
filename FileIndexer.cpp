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
#include <limits>
//struct pedge;



using namespace std;
std::map<std::string, std::map<long, std::set<std::string>>> sorteddata;// map<src,map<t,set<dst>>> such that src,dst,t is an edge

/*
 * Reads an edge list file and generate an data structure(sorteddata) which is indexed in src node and time of edge.
 * reverseEdge : Determines if the edge in file needs to read in reverse order
 */
int readFile(std::string inputFile, bool reverseEdge) {

    ifstream infile(inputFile.c_str());
    string line;
    std::vector<std::string> templine;
    long timestamp;
    string src, dst;
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = templine[0];
        dst = templine[1];
        if(src.compare(dst)!=0) {
            if (reverseEdge) {
                src = templine[1];
                dst = templine[0];
            }
            timestamp = stol(templine[2].c_str());
            sorteddata[src][timestamp].insert(dst);
        }
    }
    return 0;
}

/*
 * Returns list of edges <src,x,t> such that t is between t_s and t_end and if candidates list is provided x should be in candidates
 */
std::set<pedge> getFilteredData(string src, long t_s, long t_end, set<string> *candidates) {
    std::set<pedge> result;
    set<string>::iterator xit;

    if (sorteddata.count(src) > 0) {
        std::map<long, set<string>> m = sorteddata[src];
        for (std::map<long, set<string>>::iterator low = m.lower_bound(t_s - 1); low != m.end(); ++low) {
            long t = low->first;
            if (low->first > t_end) {
                break;
            }
            //std::cout << low->first << ' ' << low->second << std::endl;
            for (xit = low->second.begin(); xit != low->second.end(); ++xit) {
                string node = *xit;
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

std::set<pedge> getFilteredData(string src, long t_s, long t_end) {
    set<string> emptycandidates;
    return getFilteredData(src, t_s, t_end, &emptycandidates);
}

// All edges of type x,src,t_x
std::set<pedge> getFilteredData(string src, long t_s) {
    std::set<pedge> result;

    if (sorteddata.count(src) > 0) {
        if (sorteddata[src].count(t_s) > 0) {
            set<string> m = sorteddata[src][t_s];
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

long getMaxTime(string src, string dst, long t_uper) {
    long result = -1;
    if (sorteddata.count(src) > 0) {
        std::map<long, set<string>> m = sorteddata[src];
        for (std::map<long, set<string>>::iterator low = m.begin(); low != m.end(); ++low) {
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

long getMinTime(string src, string dst, long t_lower, long t_uper) {
    long result = std::numeric_limits<long>::max();
    if (sorteddata.count(src) > 0) {
        std::map<long, set<string>> m = sorteddata[src];
        for (std::map<long, set<string>>::iterator low = m.lower_bound(t_lower - 1); low != m.end(); ++low) {
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

