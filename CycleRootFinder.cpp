//
// Created by Rohit on 04-Sep-17.
//

#include "CycleRootFinder.h"
#include "FileIndexer.h"
#include "Split.h"
#include "Timer.h"
#include "MemoryMonitor.h"
#include <fstream>
#include <limits>
#include <algorithm>
#include <time.h>
#include "DetectCycle.h"

using namespace std;

/*
 * Finds the root nodes which are involved in a temporal cycle in the input interaction data and writes the results in output file.
 * Output file format: rootnode,start_time_cycle,neighbour_node resulting in cycle
 */
int findRootNodesApprox(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge) {
    map<string, set<tnode>> completeSummary;

    std::hash<std::string> str_hash;
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
    ofstream result;
    result.open(output.c_str());
    std::set<tnode>::iterator temp_tnode_it;

    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = templine[0];
        dst = templine[1];

        if (reverseEdge) {
            src = templine[1];
            dst = templine[0];
        }
        timestamp = stol(templine[2].substr(0, 10).c_str());

        if (src == dst) {
            //self loop ignored
        } else {

            //add src in the destination summary
            tnode tnodeSrc;
            tnodeSrc.vertex = src;
            tnodeSrc.time = timestamp;

            completeSummary[dst].insert(tnodeSrc);

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                for (set<tnode>::iterator it = completeSummary[src].begin();
                     it != completeSummary[src].end(); ++it) {
                    if ((it->time) > timestamp - window_bracket) {

                        if (it->vertex.compare(dst) == 0) {
                            //the destination is already in src summary hence a cycle exist
                            set<nodeid> candidates = getCandidatesSize(completeSummary[src], it->time,
                                                                       it->time + window_bracket);
                            candidates.erase(dst);
                            candidates.insert(src);
                            if (candidates.size() > 1) {//only cycles having more than 1 nodes

                                result << dst << ",";
                                result << it->time << ",";//start of cycle
                                result << timestamp << ","; //end of cycle
                                for (string x:candidates) {
                                    result << x << ",";

                                }
                                result << "\n";

                            }

                        } else {

                            completeSummary[dst].insert(*it);

                        }
                    } else {
                        completeSummary[src].erase(it);



                    }
                }
            }


        }
        count++;
        if (count % cleanUpLimit == 0) {
            //do cleanup

            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            int cleanupsize = cleanup(&completeSummary, timestamp, window_bracket);
            std::cout << "finished parsing, count," << count << "," << parseTime << "," << getMem();
            cout << ",summary size," << completeSummary.size();
            cout << ",memory," << getMem();
            cout << " ,delete count," << cleanupsize;
            std::cout << " ,clean time," << timer.LiveElapsedSeconds() - ptime << std::endl;
        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;

    timer.Stop();

    result.close();
    return 0;
}

int cleanup(map<string, set<tnode>> *completeSummary, long timestamp, long window_bracket) {
    int size = 0;
    string src = "";
    vector<string> deletelist;

    for (map<string, set<tnode>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {


        size = it->second.size();
        src = it->first;
        if (size > 0) {
            for (set<tnode>::iterator itinner = it->second.begin();
                 itinner != it->second.end(); ++itinner) {
                if (itinner->time < timestamp - window_bracket) {
                    it->second.erase(itinner);

                }

            }

        } else {

            //completeSummary->erase(it);
            deletelist.push_back(it->first);

        }


    }
    for (auto x: deletelist) {
        completeSummary->erase(x);
    }

    return deletelist.size();
}

set<string> getCandidatesSize(set<tnode> summary, long t_s, long t_e) {
    set<string> candidates;
    long time;
    for (set<tnode>::iterator it = summary.begin(); it != summary.end(); ++it) {

        if (it->time >= t_s && time < t_e) {

            candidates.insert(it->vertex);
        }
    }
    return candidates;
}