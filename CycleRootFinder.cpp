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
    map<string, map<long, set<long>>> completeSummary;

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
            completeSummary[dst][-1 * timestamp].insert(str_hash(src));

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                for (map<long, set<long>>::iterator it = completeSummary[src].begin();
                     it != completeSummary[src].end(); ++it) {
                    if ((-1 * it->first) > timestamp - window_bracket) {

                        if (it->second.count(str_hash(dst)) > 0) {
                            //the destination is already in src summary hence a cycle exist
                            int candidate_size = getCandidatesSize(completeSummary[src], -1 * it->first,
                                                                   (-1 * it->first) + window_bracket);
                            if (candidate_size > 1) {
                                result << dst << ",";
                                result << (-1 * it->first) << ",";//start of cycle
                                result << timestamp << ","; //end of cycle

                                result << "\n";

                            }
                            // add other in the summary
                            for (auto x:it->second) {
                                if (x != str_hash(dst)) {
                                    completeSummary[dst][it->first].insert(x);
                                }
                            }
                        } else {
                            completeSummary[dst][it->first].insert(it->second.begin(), it->second.end());
                        }
                    } else {
                        completeSummary[src].erase(it, completeSummary[src].end());
                        break;


                    }
                }
            }


        }
        count++;
        if (count % cleanUpLimit == 0) {
            //do cleanup

            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            int cleanupsize=cleanup(&completeSummary, timestamp, window_bracket);
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

int cleanup(map<string, map<long, set<long>>> *completeSummary, long timestamp, long window_bracket) {
    int size = 0;
    string src = "";
    vector<string> deletelist;
    int max_value_size = 0;
    int max_set_size = 0;
    for (map<string, map<long, set<long>>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {

        size = it->second.size();
        src = it->first;
        if (size > 0) {
            for (map<long, set<long>>::iterator itinner = it->second.begin();
                 itinner != it->second.end(); ++itinner) {
                if ((-1 * itinner->first) < timestamp - window_bracket) {
                    it->second.erase(itinner, it->second.end());
                    break;
                }
                if (itinner->second.size() > max_set_size) {
                    max_set_size = itinner->second.size();
                }
            }
            if (it->second.size() > max_value_size) {
                max_value_size = it->second.size();
            }
        } else {

            //completeSummary->erase(it);
            deletelist.push_back(it->first);

        }

    }
    for (auto x: deletelist) {
        completeSummary->erase(x);
    }
    cout<<"max # of times "<<max_value_size<<" max set size "<<max_set_size<<endl;
    return deletelist.size();
}

int getCandidatesSize(map<long, set<long>> summary, long t_s, long t_e) {
    set<long> candidates;
    long time;
    for (map<long, set<long>>::iterator it = summary.begin(); it != summary.end(); ++it) {
        time = -1 * it->first;
        if (time >= t_s && time < t_e) {

            candidates.insert(it->second.begin(), it->second.end());
        }
    }
    return candidates.size();
}