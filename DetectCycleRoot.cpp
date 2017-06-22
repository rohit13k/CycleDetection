//
// Created by Rohit on 21-Jun-17.
//

#include "DetectCycleRoot.h"
#include "countCycleFrequency.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Split.h"
#include "Timer.h"
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
                            //cycle found get all candidates
                            //  findCandidateWhileUpdate(src, dst, &completeSummary, timestamp, window_bracket,&timeGroupIt);
                            /*
                            for(timeGroup::reverse_iterator newit=completeSummary.find(src)->second.rbegin();newit!=completeSummary.find(src)->second.rend();++newit){
                                for (set<nodeid>::iterator iterator1 = newit->second.begin(); iterator1 != newit->second.end(); ++iterator1) {
                                    if((*iterator1).compare(dst)!=0){
                                        rootNodes[dst][temptime].insert(*iterator1);

                                    }
                                }
                            }
                            */
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

void copy(string fromNode, string toNode, map<nodeid, map<nodeid, long>> *completeSummary, long currenttime,
          long window_bracket, long candidateTime) {

    set<nodeid> candidateset;
    for (map<nodeid, long>::iterator it = completeSummary->find(fromNode)->second.begin();
         it != completeSummary->find(fromNode)->second.end(); ++it) {
        //if out of window remove it,
        if (currenttime - it->second > window_bracket) {
            (*completeSummary)[fromNode].erase(it);
        } else {
            //if already present in dst summary update time if greater
            if ((*completeSummary)[toNode].count(it->first) > 0) {
                if ((*completeSummary)[toNode][it->first] > it->second) {
                    (*completeSummary)[toNode][it->first] = it->second;
                }
            } else {
                (*completeSummary)[toNode][it->first] = it->second;
            }
        }

    }
}

set<nodeid>
findCandidateWhileUpdate(string fromNode, string toNode, map<nodeid, timeGroup> *completeSummary, long currenttime,
                         long window_bracket, timeGroup::iterator *it) {
    set<nodeid> candidateset;

    if (*it != (*completeSummary).find(fromNode)->second.end()) {
        long newtime = (*it)->first;
        if (currenttime - (*it)->first > window_bracket) {
            // if reached time which is out of window delete anything from now till end and return as no new candidate is possible
            completeSummary->find(fromNode)->second.erase((*it), completeSummary->find(fromNode)->second.end());
            return candidateset;
        } else {
            //all elements in this time is candidate
            candidateset.insert((*it)->second.begin(), (*it)->second.end());
            *it++;
            set<nodeid> newcandidates = findCandidateWhileUpdate(fromNode, toNode, completeSummary, currenttime,
                                                                 window_bracket, it);
            candidateset.insert(newcandidates.begin(), newcandidates.end());
            if ((*it)->second.count(toNode) == 0) {
                // to node is present hence start a new cycle

                candidateset.erase(toNode);
                rootNodes[toNode][(*it)->first] = candidateset;

            }

        }

    }

    return candidateset;

}
