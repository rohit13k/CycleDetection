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
#include "bloom_filter.hpp"

using namespace std;

/*
 * Finds the root nodes which are involved in a temporal cycle in the input interaction data and writes the results in output file.
 * Output file format: rootnode,start_time_cycle,neighbour_node resulting in cycle
 */
int findRootNodesNew(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge) {
    map<int, map<int, int>> completeSummary;
    map<int, map<int, int>>::iterator src_iterator;
    map<int, map<int, int>>::iterator dst_iterator;


    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    int src, dst;

    string line;
    Platform::Timer timer;
    timer.Start();
    int timestamp;

    int count = 0;

    int window_bracket = window * 60 * 60;
    double ptime = 0.0;

    ofstream result;
    result.open(output.c_str());

    vector<string> all_data;
    while (infile >> line) {
        all_data.push_back(line);
    }
    cout << all_data.size() << endl;
    for (int j = all_data.size() - 1; j >= 0; j--) {
        line = all_data[j];
        templine = Tools::Split(line, ',');
        src = stoi(templine[0]);
        dst = stoi(templine[1]);


        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        timestamp = stoi(templine[2]);

        if (src == dst) {
            //self loop ignored
        } else {
            completeSummary[src][dst] = timestamp;
            src_iterator = completeSummary.find(src);
            dst_iterator = completeSummary.find(dst);


            map<int, int> &src_summary = src_iterator->second;
            //add src in the destination summary
            /*
              tnode tnodeSrc;
              tnodeSrc.vertex = src;
              tnodeSrc.time = timestamp;
              */


            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (dst_iterator != completeSummary.end()) {
                map<int, int> &dst_summary = dst_iterator->second;
                for (map<int, int>::iterator it = dst_summary.begin();
                     it != dst_summary.end(); ++it) {
                    if ((it->second - timestamp) < window_bracket) {

                        if (it->first == src) {
                            //the destination is already in src summary hence a cycle exist
                            set<int> candidates = getCandidates(&dst_summary, timestamp, timestamp + window_bracket);
                            candidates.erase(src);
                            candidates.insert(dst);
                            if (candidates.size() > 1) {//only cycles having more than 1 nodes

                                result << src << ",";
                                result << timestamp << ",";//start of cycle
                                result << timestamp + window_bracket << ","; //end of cycle
                                for (int x:candidates) {
                                    result << x << ",";

                                }
                                result << "\n";

                            }

                        } else {

                            if (src_summary.count(it->first) > 0) {
                                if (src_summary[it->first] > it->second) {
                                    src_summary[it->first] = it->second;
                                }
                            } else {
                                src_summary[it->first] = it->second;
                            }


                        }
                    } else {
                        dst_summary.erase(it);


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

int cleanup(map<int, map<int, int>> *completeSummary, int timestamp, int window_bracket) {
    int size = 0;
    string src = "";
    vector<int> deletelist;
    int max_size = 0;
    int min_size = 99999999;
    int total_set_size = 0;
    int total = completeSummary->size();
    for (map<int, map<int, int>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {
        size = it->second.size();
        src = it->first;
        total_set_size = total_set_size + size;
        if (size > max_size) {
            max_size = size;
        }
        if (size < min_size) {
            min_size = size;
        }
        if (size > 0) {
            for (map<int, int>::iterator itinner = it->second.begin();
                 itinner != it->second.end(); ++itinner) {
                if (itinner->second - timestamp > window_bracket) {
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
    float avg_set_size = (total_set_size + 0.0) / total;
    cout << "max set size," << max_size << ",min set size," << min_size << ",avg," << avg_set_size << endl;
    return deletelist.size();
}

set<int> getCandidates(map<int, int> *summary, int t_s, int t_e) {
    set<int> candidates;

    for (map<int, int>::iterator it = summary->begin(); it != summary->end(); ++it) {

        if (it->second >= t_s && it->second < (t_e + 1)) {

            candidates.insert(it->first);
        }
    }
    return candidates;
}

int findRootNodesApprox(std::string input, std::string output, int window, int cleanUpLimit, bool reverseEdge) {
    map<int, bloom_filter> completeSummary;
    map<int, bloom_filter>::iterator src_iterator;
    map<int, bloom_filter>::iterator dst_iterator;
    map<int, int> node_update_time;
    bloom_parameters parameters;

    // How many elements roughly do we expect to insert?
    parameters.projected_element_count = 1000;

    // Maximum tolerable false positive probability? (0,1)
    parameters.false_positive_probability = 0.0001; // 1 in 10000

    // Simple randomizer (optional)
    parameters.random_seed = 0xA5A5A5A5;
    parameters.compute_optimal_parameters();

    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    int src, dst;

    string line;
    Platform::Timer timer;
    timer.Start();
    int timestamp;

    int count = 0;

    int window_bracket = window * 60 * 60;
    double ptime = 0.0;

    ofstream result;
    result.open(output.c_str());

    vector<string> all_data;
    while (infile >> line) {
        all_data.push_back(line);
    }
    cout << all_data.size() << endl;
    for (int j = all_data.size() - 1; j >= 0; j--) {
        line = all_data[j];
        templine = Tools::Split(line, ',');
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        timestamp = stoi(templine[2]);
        if (src == dst) {
            //self loop ignored
        } else {

            if (completeSummary.count(src) > 0) {
                //Instantiate Bloom Filter
                bloom_filter filter(parameters);
                filter.insert(dst);
                completeSummary[src] = filter;

            } else {
                completeSummary[src].insert(dst);

            }
            node_update_time[src] = timestamp;
            src_iterator = completeSummary.find(src);
            dst_iterator = completeSummary.find(dst);
            bloom_filter &src_summary = src_iterator->second;
            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (dst_iterator != completeSummary.end()) {
                if (dst_iterator->second.contains(src)) {

                    result << src << ",";
                    result << timestamp << ",";//start of cycle
                    result << timestamp + window_bracket; //end of cycle

                    result << "\n";

                }
                src_summary |= dst_iterator->second;
            }
        }


        count++;
        if (count % cleanUpLimit == 0) {
            //do cleanup

            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            int cleanupsize = cleanup(&completeSummary, &node_update_time, timestamp, window_bracket);
            std::cout << "finished parsing, count," << count << "," << parseTime << "," << getMem();
            cout << ",summary size," << completeSummary.size();
            cout << ",memory," << getMem();
            cout << " ,delete count," << cleanupsize;
            std::cout << " ,clean time," << timer.LiveElapsedSeconds() - ptime << std::endl;
        }
    }


    std::cout << "finished parsing all " << timer.LiveElapsedSeconds() << std::endl;
    timer.Stop();
    result.close();
    return 0;
}

int
cleanup(map<int, bloom_filter> *completeSummary, map<int, int> *node_update_time, int timestamp, int window_bracket) {
    int size = 0;
    string src = "";
    vector<int> deletelist;

    for (map<int, int>::iterator it = node_update_time->begin();
         it != node_update_time->end(); ++it) {

        if (it->second - timestamp > window_bracket) {
            deletelist.push_back(it->first);
        }
    }
    for (auto x: deletelist) {
        completeSummary->erase(x);
    }

    return deletelist.size();
}