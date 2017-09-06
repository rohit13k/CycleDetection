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
    map<string, map<string,long>> completeSummary;

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
    vector<string> all_data;
    while (infile >> line) {
        all_data.push_back(line);
    }
    cout<<all_data.size()<<endl;
    for(int j=all_data.size()-1;j>=0;j--) {
        line=all_data[j];
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
          /*
            tnode tnodeSrc;
            tnodeSrc.vertex = src;
            tnodeSrc.time = timestamp;
            */
            completeSummary[src][dst]=timestamp;

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(dst) > 0) {
                for (map<string,long>::iterator it = completeSummary[dst].begin();
                     it != completeSummary[dst].end(); ++it) {
                    if ((it->second-timestamp) <  window_bracket) {

                        if (it->first.compare(src) == 0) {
                            //the destination is already in src summary hence a cycle exist
                            set<nodeid> candidates = getCandidates(completeSummary[dst], timestamp, timestamp + window_bracket);
                            candidates.erase(src);
                            candidates.insert(dst);
                            if (candidates.size() > 1) {//only cycles having more than 1 nodes

                                result << src << ",";
                                result << timestamp << ",";//start of cycle
                                result << timestamp + window_bracket << ","; //end of cycle
                                for (string x:candidates) {
                                    result << x << ",";

                                }
                                result << "\n";

                            }

                        } else {

                            if(completeSummary[src].count(it->first)>0){
                                if(completeSummary[src][it->first]>it->second){
                                    completeSummary[src][it->first]=it->second;
                                }
                            }else{
                                completeSummary[src][it->first]=it->second;
                            }


                        }
                    } else {
                        completeSummary[dst].erase(it);


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

int cleanup(map<string, map<string,long>> *completeSummary, long timestamp, long window_bracket) {
    int size = 0;
    string src = "";
    vector<string> deletelist;
    int max_size = 0;
    for (map<string, map<string,long>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {


        size = it->second.size();
        src = it->first;
        if (size > max_size) {
            max_size = size;
        }
        if (size > 0) {
            for (map<string,long>::iterator itinner = it->second.begin();
                 itinner != it->second.end(); ++itinner) {
                if (itinner->second- timestamp>  window_bracket) {
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
    cout << "max set size " << max_size<<endl;
    return deletelist.size();
}

set<string> getCandidates(map<string,long> summary, long t_s, long t_e) {
    set<string> candidates;

    for (map<string,long>::iterator it = summary.begin(); it != summary.end(); ++it) {

        if (it->second >= t_s && it->second < (t_e+1)) {

            candidates.insert(it->first);
        }
    }
    return candidates;
}