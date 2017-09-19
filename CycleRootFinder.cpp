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
    map<int, map<int, set<int>>> completeSummary;


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


    while (infile >> line) {
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

            //add src in the destination summary
            completeSummary[dst][-1 * timestamp].insert(src);

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                vector<string> cycles = updateSummaries(&completeSummary, timestamp,
                                                        window_bracket, src, dst);
                for (string c:cycles) {
                    result << c << "\n";
                }
            }


        }
        count++;
        if (count % cleanUpLimit == 0) {
            //do cleanup

            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();
            std::cout << "finished parsing, count," << count << "," << parseTime << "," << getMem();
            cout << ",summary size," << completeSummary.size();
            cout << ",memory," << getMem();
            cout << " ,delete count," << cleanup(&completeSummary, timestamp, window_bracket);
            std::cout << " ,clean time," << timer.LiveElapsedSeconds() - ptime << std::endl;
        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    timer.Stop();

    result.close();
    return 0;
}

vector<string>
updateSummaries(map<int, map<int, set<int>>> *completeSummary, int timestamp,
                int window_bracket, int src, int dst) {
    map<int, set<int>> &src_summary = (*completeSummary)[src];
    map<int, set<int>> &dst_summary = (*completeSummary)[dst];

    vector<string> cycles;
    for (map<int, set<int>>::iterator it = src_summary.begin();
         it != src_summary.end(); ++it) {
        if ((-1 * it->first) > timestamp - window_bracket) {

            if (it->second.count(dst) > 0) {
                //the destination is already in src summary hence a cycle exist
                set<int> candidates = getCandidates(src_summary, -1 * it->first,
                                                    (-1 * it->first) + window_bracket);
                candidates.erase(dst);
                candidates.insert(src);
                if (candidates.size() > 1) {//only cycles having more than 1 nodes

                    string result = to_string(dst) + ",";
                    result = result + to_string(-1 * it->first) + ",";//start of cycle
                    result = result + to_string(timestamp) + ","; //end of cycle
                    for (int x:candidates) {
                        result = result + to_string(x) + ",";

                    }

                    cycles.push_back(result);

                }
                // add other in the summary
                for (auto x:it->second) {
                    if (x != dst) {
                        dst_summary[it->first].insert(x);
                    }
                }
            } else {
                dst_summary[it->first].insert(it->second.begin(), it->second.end());
            }
        } else {
            src_summary.erase(it, src_summary.end());
            break;


        }
    }//forloop
    return cycles;
}

int cleanup(map<int, map<int, set<int>>> *completeSummary, int timestamp, int window_bracket) {
    int size = 0;
    string src = "";
    vector<int> deletelist;
    for (map<int, map<int, set<int>>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {

        size = it->second.size();
        src = it->first;
        if (size > 0) {
            for (map<int, set<int>>::iterator itinner = it->second.begin();
                 itinner != it->second.end(); ++itinner) {
                if ((-1 * itinner->first) < timestamp - window_bracket) {
                    it->second.erase(itinner, it->second.end());
                    break;
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

int cleanupNew(map<int, map<int, int>> *completeSummary, int timestamp, int window_bracket, bool forward) {
    int size = 0;
    string src = "";
    vector<int> deletelist;
    int max_size = 0;
    int min_size = 99999999;
    int total_set_size = 0;
    int total = completeSummary->size();
    int time_diff;
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
                if (forward) {
                    time_diff = timestamp - itinner->second;
                } else {
                    time_diff = itinner->second - timestamp;
                }
                if (time_diff > window_bracket) {
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

set<int> getCandidatesNew(map<int, int> *summary, int t_s, int t_e) {
    set<int> candidates;

    for (map<int, int>::iterator it = summary->begin(); it != summary->end(); ++it) {

        if (it->second >= t_s && it->second < (t_e + 1)) {

            candidates.insert(it->first);
        }
    }
    return candidates;
}

set<int> getCandidates(map<int, set<int>> summary, int t_s, int t_e) {
    set<int> candidates;
    long time;
    for (map<int, set<int>>::iterator it = summary.begin(); it != summary.end(); ++it) {
        time = -1 * it->first;
        if (time >= t_s && time < t_e) {

            candidates.insert(it->second.begin(), it->second.end());
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

            if (completeSummary.count(src) == 0) {
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
                if (node_update_time[dst] - timestamp > window_bracket) {
                    dst_iterator->second.clear();
                } else {
                    if (dst_iterator->second.contains(src)) {

                        if (dst_iterator->second.element_count() > 1) {
                            result << src << ",";
                            result << timestamp << ",";//start of cycle
                            result << dst; //end of cycle

                            result << "\n";
                        }

                    }
                    src_summary |= dst_iterator->second;
                    src_summary.update_element_count(dst_iterator->second.element_count());
                }
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


int findRootNodesApproxBothDirection(std::string input, std::string output, int window, int cleanUpLimit,
                                     bool reverseEdge) {
    map<int, bloom_filter> completeSummary;

    map<int, int> node_update_time;
    map<int, set<pair<int, int>>> rootnode_end_time_set;
    map<int, set<pair<int, int>>>::iterator rootnode_end_time_set_itr;
    bloom_parameters parameters;
    pair<int, pair<int, int>> root_candidate;
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

        templine = Tools::Split(line, ',');
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        timestamp = stoi(templine[2]);

        all_data.push_back(line);
        root_candidate = updateSummary(dst, src, timestamp, window_bracket, &completeSummary, parameters,
                                       &node_update_time);
        if (root_candidate.first != 0) {
            rootnode_end_time_set[root_candidate.first].insert(root_candidate.second);
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

    int end_time;

    int end_neighbour;
    completeSummary.clear();
    node_update_time.clear();
    count = 0;
    cout << rootnode_end_time_set.size() << endl;
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

        root_candidate = updateSummary(src, dst, timestamp, window_bracket, &completeSummary, parameters,
                                       &node_update_time);
        if (root_candidate.first != 0) {
//check if there is a valid end for this start node.
            rootnode_end_time_set_itr = rootnode_end_time_set.find(root_candidate.first);
            if (rootnode_end_time_set_itr != rootnode_end_time_set.end()) {

                set<pair<int, int>> &possible_end_time_set = rootnode_end_time_set_itr->second;

                for (set<pair<int, int>>::iterator it = possible_end_time_set.begin();
                     it != possible_end_time_set.end(); ++it) {
                    end_time = it->second;
                    end_neighbour = it->first;
                    if (end_time - timestamp > 0 & end_time - timestamp < window_bracket) {
                        if (dst != end_neighbour) {
                            result << src << ",";
                            result << timestamp << ",";//start of cycle
                            result << dst; //end of cycle

                            result << "\n";
                            break;
                        }
                    }
                }
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

pair<int, pair<int, int>>
updateSummary(int src, int dst, int timestamp, int window_bracket, map<int, bloom_filter> *summary,
              bloom_parameters parameters, map<int, int> *update_time) {
    map<int, bloom_filter>::iterator src_iterator;
    map<int, bloom_filter>::iterator dst_iterator;
    map<int, bloom_filter> &completeSummary = *summary;
    map<int, int> &node_update_time = *update_time;
    pair<int, pair<int, int>> result;

    if (src == dst) {
        //self loop ignored
    } else {

        if (completeSummary.count(src) == 0) {
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
        if (dst_iterator != completeSummary.end() & node_update_time.count(dst) > 0) {
            if (abs(node_update_time[dst] - timestamp) > window_bracket) {
                dst_iterator->second.clear();
            } else {
                if (dst_iterator->second.contains(src)) {

                    if (dst_iterator->second.element_count() > 1) {
                        result = make_pair(src, make_pair(dst, timestamp));

                    }

                }
                src_summary |= dst_iterator->second;
                src_summary.update_element_count(dst_iterator->second.element_count());
            }
        }
    }

    return result;
}

int
cleanup(map<int, bloom_filter> *completeSummary, map<int, int> *node_update_time, int timestamp, int window_bracket) {
    int size = 0;
    string src = "";
    vector<int> deletelist;

    for (map<int, int>::iterator it = node_update_time->begin();
         it != node_update_time->end(); ++it) {

        if (abs(it->second - timestamp) > window_bracket) {
            deletelist.push_back(it->first);
        }
    }
    for (auto x: deletelist) {
        completeSummary->erase(x);
    }

    return deletelist.size();
}

int findCandidateFromApprox(std::string input, string root, std::string output, int window, int cleanUpLimit,
                            bool reverseEdge) {
    ifstream root_file(root.c_str());
    map<int, int> watchlist;
    vector<string> all_root;
    map<int, map<int, set<int>>> completeSummary;
    string line;
    while (root_file >> line) {
        all_root.push_back(line);
    }
    ifstream input_file(input.c_str());
    int all_root_position = all_root.size() - 1;
    int all_root_old_position = 0;
    int root_node;
    int start_time;
    int root_neighbour;
    int src, dst, timestamp;
    std::vector<std::string> templine;
    int window_bracket = window * 60 * 60;

    ofstream result;
    int count = 0;
    result.open(output.c_str());
    Platform::Timer timer;
    double ptime = 0.0;
    timer.Start();
    while (input_file >> line) {

        if ((all_root_old_position != all_root_position) & all_root_position >= 0) {
            templine = Tools::Split(all_root[all_root_position], ',');
            root_node = stoi(templine[0]);
            start_time = stoi(templine[1]);
            root_neighbour = stoi(templine[2]);
            all_root_old_position = all_root_position;
        }

        templine = Tools::Split(line, ',');
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        timestamp = stoi(templine[2]);
        if (src != dst) {//avoid self loop

            //if last root is processed more then window time ago exit
            if (all_root_position < 0) {
                if (timestamp - start_time > window_bracket) {
                    break;
                }
            }
            //if the edge from root node is found in data file
            if ((src == root_node & dst == root_neighbour & timestamp == start_time)) {
                //add destination node in watchlist or update its time
                watchlist[dst] = timestamp;
                //add src in the complete summary of dst
                completeSummary[dst][-1 * timestamp].insert(src);

                all_root_position--;
            } else if (watchlist.count(src) > 0) { //else if src is in watchlist add dst also in watch list
                if (timestamp - watchlist[src] < window_bracket) {
                    watchlist[dst] = timestamp;
                    completeSummary[dst][-1 * timestamp].insert(src);

                } else {
                    watchlist.erase(src);
                    completeSummary.erase(src);
                }
            }

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                vector<string> cycles = updateSummaries(&completeSummary, timestamp,
                                                        window_bracket, src, dst);
                for (string c:cycles) {
                    result << c << "\n";
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
    }//end of while loop
}