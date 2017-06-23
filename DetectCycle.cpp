//
// Created by Rohit on 21-Jun-17.
//

#include "DetectCycle.h"
#include "FileIndexer.h"
#include "Split.h"
#include "Timer.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <set>

using namespace std;

map<nodeid, timeGroup> rootNodes;

int findRootNodes(std::string input, std::string output, int window, bool timeInMsec, int cleanUpLimit) {
    map<nodeid, timeGroup> completeSummary;

    map<nodeid, timeGroup>::iterator it;
    timeGroup::iterator timeGroupIt;
    std::set<nodeid>::iterator itnode;
    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    string src, dst;
    long negativeTimestamp;
    string line;
    Platform::Timer timer;
    timer.Start();
    long timestamp;
    long temptime = 0l;
    int count = 0;
    bool cycleFound = false;
    long window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string tempnode = "";

    if (timeInMsec) {
        window_bracket = window_bracket * 1000;
    }

    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = templine[0];
        dst = templine[1];
        timestamp = stol(templine[2].c_str());
        negativeTimestamp = -1 * timestamp;
        if (src.compare(dst) == 0) {
            //self loop ignored
        } else {
            //if src summary exist transfer it to be if in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                set<nodeid> candidateset;
                candidateset.insert(src);

                for (timeGroupIt = completeSummary.find(src)->second.begin();
                     timeGroupIt != completeSummary.find(src)->second.end(); ++timeGroupIt) {
                    temptime = -1 * timeGroupIt->first;
                    if (timestamp - temptime > window_bracket) {
                        completeSummary.find(src)->second.erase(timeGroupIt, completeSummary.find(src)->second.end());
                        break;
                    }
                    for (itnode = timeGroupIt->second.begin(); itnode != timeGroupIt->second.end(); ++itnode) {
                        tempnode = *itnode;
                        if (tempnode.compare(dst) == 0) {
                            
                            cycleFound = true;

                        } else {
                            candidateset.insert(tempnode);
                            completeSummary[dst][timeGroupIt->first].insert(tempnode);
                        }
                    }
                    if (cycleFound) {
                        cycleFound = false;
                        if (candidateset.size() > 1)
                            rootNodes[dst][-1 * timeGroupIt->first] = candidateset;
                    }
                }
            }
            completeSummary[dst][negativeTimestamp].insert(src);

        }
        count++;
        if (count % cleanUpLimit == 0) {
            //do cleanup
            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            for (it = completeSummary.begin(); it != completeSummary.end(); ++it) {
                for (timeGroupIt = completeSummary.find(it->first)->second.begin();
                     timeGroupIt != completeSummary.find(it->first)->second.end(); ++timeGroupIt) {
                    //  std::cout <<  timestamp - (-1 * itTimedSet->first)<< std::endl;
                    if (timestamp - (-1 * timeGroupIt->first) > window_bracket) {
                        completeSummary.find(it->first)->second.erase(timeGroupIt, completeSummary.find(
                                it->first)->second.end());
                        break;
                    }
                }
            }
            double eraseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            std::cout << "finished parsing, count," << count << "," << parseTime << "," << eraseTime << std::endl;

        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    timer.Stop();
    ofstream result;
    result.open(output.c_str());
    for (std::map<nodeid, timeGroup>::iterator rootNodesIt = rootNodes.begin();
         rootNodesIt != rootNodes.end(); ++rootNodesIt) {


        for (timeGroupIt = rootNodesIt->second.begin(); timeGroupIt != rootNodesIt->second.end(); ++timeGroupIt) {
            result << rootNodesIt->first << ",";
            result << timeGroupIt->first << ",";
            for (string x:timeGroupIt->second) {
                result << x << ",";
            }
            result << "\n";
        }
    }
    result.close();
    return 0;
}

int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window, bool timeInMsec,
                 bool usingGlobalBlock) {
    long window_bracket = window * 60 * 60;
    double ptime = 0.0;
    if (timeInMsec) {
        window_bracket = window_bracket * 1000;
    }
    string line;
    Platform::Timer timer;
    timer.Start();
    readFile(dataFile);
    ptime = timer.LiveElapsedSeconds();
    std::cout << "finished reading " << ptime
              << std::endl;

    std::vector<std::string> templine;
    ifstream infile(rootNodeFile.c_str());
    string rootnode;
    long t_s;
    int i = 0;
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        set<string> candidateset;
        rootnode = templine[0];
        t_s = stol(templine[1].c_str());
        for (i = 2; i < templine.size(); i++) {
            candidateset.insert(templine[i]);
        }
        candidateset.insert(rootnode);
        findCycle(rootnode, t_s, &candidateset, window_bracket);
    }
}

void findCycle(std::string rootNode, long t_s, std::set<std::string> *candidates, long window_bracket) {
    set<pedge> neighbours = getFilteredData(rootNode, t_s);
    vector<std::string> initialpath;
    set<std::string> seen;
    for (set<pedge>::iterator eit = neighbours.begin(); eit != neighbours.end(); ++eit) {
        initialpath.clear();
        seen.clear();
        initialpath.push_back(rootNode);
        initialpath.push_back(eit->toVertex);
        seen.insert(eit->toVertex);
        if (findTemporalPath(eit->toVertex, rootNode, t_s, t_s + window_bracket, &initialpath, seen, candidates)) {
            std::cout << "Found cycle: " << initialpath.size() << " : " ;
            for (int i = 0; i < initialpath.size(); i++) {
                std::cout << "->" << initialpath[i];
            }
            cout << endl;
        }
    }

}

bool findTemporalPath(std::string src, std::string dst, long t_s, long t_end, vector<std::string> *path_till_here,
                      set<string> seen, std::set<std::string> *candidates) {

    vector<string> path_till_here_bkp;
    for (int i = 0; i < path_till_here->size(); i++) {
        path_till_here_bkp.push_back((*path_till_here)[i]);
    }

    set<pedge> Y = getFilteredData(src, t_s, t_end, candidates);
    bool found = false;
    for (set<pedge>::iterator edgeIt = Y.begin(); edgeIt != Y.end(); ++edgeIt) {
        if (edgeIt->toVertex.compare(dst) == 0) {
            path_till_here->push_back(edgeIt->toVertex);
            return true;
        } else if (seen.count(edgeIt->toVertex) == 0) {
            seen.insert(edgeIt->toVertex);
            path_till_here->push_back(edgeIt->toVertex);
            found = findTemporalPath(edgeIt->toVertex, dst, edgeIt->time + 1, t_end, path_till_here, seen, candidates);

            if (found) {
                std::cout << "Found cycle: " << path_till_here->size() << " : " ;
                for (int i = 0; i < path_till_here->size(); i++) {
                    std::cout << "->" << (*path_till_here)[i];

                }
                path_till_here->clear();
                for(int i=0;i<path_till_here_bkp.size();i++){
                    path_till_here->push_back(path_till_here_bkp[i]);
                }
                cout << endl;
                found=false;
            } else {
                return found;
            }

        }
    }
    return found;

}
