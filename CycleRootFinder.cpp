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
            cout << ",memory after cleanup," << getMem();
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
                    dst_iterator->second.clean();
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

set<approxCandidatesNew>
findRootNodesApproxBothDirectionWithSerialization(std::string input, std::string output, int window, int cleanUpLimit,
                                    bool reverseEdge,std::string tempFolder) {

    map<int, bloom_filter> completeSummary;
    //map<root,map<<t_start,t_end>,<dst,approx candidateset>>>
    map<int, map<cycle_time, map<int, bloom_filter>>> root_candidate_approx;
    map<int, int> node_update_time;
    map<int, set<endNodeNew>> rootnode_end_time_set;
    map<int, set<endNodeNew>>::iterator rootnode_end_time_set_itr;
    bloom_parameters parameters;
    pair<int, pair<int, int>> root_neigbhour_time;
    // How many elements roughly do we expect to insert?
    parameters.projected_element_count = 1000;

    // Maximum tolerable false positive probability? (0,1)
    parameters.false_positive_probability = 0.0001; // 1 in 10000

    // Simple randomizer (optional)
    parameters.random_seed = 0xA5A5A5A5;
    parameters.compute_optimal_parameters();

    bloom_filter::optimal_param(parameters);

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

    // ofstream result;
    // result.open(output.c_str());

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
        root_neigbhour_time = updateSummary(dst, src, timestamp, window_bracket, &completeSummary, parameters,
                                            &node_update_time);
        if (root_neigbhour_time.first != 0) {
            endNodeNew en;
            en.node_id = root_neigbhour_time.second.first;
            en.end_time = root_neigbhour_time.second.second;
            rootnode_end_time_set[root_neigbhour_time.first].insert(en);
            string binfile=tempFolder+to_string(en.node_id)+"_"+to_string( en.end_time )+".bin";
            std::ofstream os(binfile, std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(completeSummary[dst]);
            os.flush();
            os.close();
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

    cout << "Memory after first pass: " << getMem() << std::endl;
    int end_neighbour;
    completeSummary.clear();
    node_update_time.clear();

    cout << "Memory after first pass after clear: " << getMem() << std::endl;
    count = 0;
    cout << rootnode_end_time_set.size() << endl;
    set<endNodeNew>::iterator possible_end_time_set_it;

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

        root_neigbhour_time = updateSummary(src, dst, timestamp, window_bracket, &completeSummary, parameters,
                                            &node_update_time);
        if (root_neigbhour_time.first != 0) {
//check if there is a valid end for this start node.
            rootnode_end_time_set_itr = rootnode_end_time_set.find(root_neigbhour_time.first);
            if (rootnode_end_time_set_itr != rootnode_end_time_set.end()) {

                set<endNodeNew> &possible_end_time_set = rootnode_end_time_set_itr->second;

                for (possible_end_time_set_it = possible_end_time_set.begin();
                     possible_end_time_set_it != possible_end_time_set.end(); ++possible_end_time_set_it) {
                    end_time = possible_end_time_set_it->end_time;
                    end_neighbour = possible_end_time_set_it->node_id;

                    if (end_time - timestamp > 0 & end_time - timestamp < window_bracket) {
                        if (dst != end_neighbour) {
                            //    result << src << ",";
                            //    result << timestamp << ",";//start of cycle
                            //    result << dst; //end of cycle

                            //    result << "\n";
                            string binfile=tempFolder+to_string(possible_end_time_set_it->node_id)+"_"+to_string(possible_end_time_set_it->end_time)+".bin";
                            std::ifstream INFILE(binfile, std::ios::binary);
                            cereal::BinaryInputArchive iarchive(INFILE);
                            bloom_filter old_bloom;
                            iarchive(old_bloom);
                            //  old_bloom = possible_end_time_set_it->candidates;
                            old_bloom &= completeSummary[dst];
                            //create candidates for cycles
                            if (root_candidate_approx.count(src) > 0) {

                                bool added = false;
                                for (map<cycle_time, map<int, bloom_filter>>::iterator cycle_itr = root_candidate_approx[src].begin();
                                     cycle_itr != root_candidate_approx[src].end(); cycle_itr++) {
                                    if (abs(cycle_itr->first.start_time - timestamp) < window_bracket) {
                                        int min_ts = min(cycle_itr->first.start_time, timestamp);
                                        int max_te = max(cycle_itr->first.end_time, end_time);
                                        if (max_te - min_ts < window_bracket) {
                                            cycle_time old_ct = cycle_itr->first;
                                            cycle_time new_ct;
                                            new_ct.start_time = min_ts;
                                            new_ct.end_time = max_te;
                                            if (new_ct == old_ct) {

                                            } else {
                                                map<int, bloom_filter> &temp = cycle_itr->second;
                                                temp[dst] = old_bloom;
                                                root_candidate_approx[src][new_ct] = temp;
                                                root_candidate_approx[src].erase(old_ct);
                                            }
                                            added = true;
                                            break;

                                        }
                                    }
                                }
                                if (!added) {
                                    cycle_time ct;
                                    ct.start_time = timestamp;
                                    ct.end_time = end_time;
                                    root_candidate_approx[src][ct][dst] = old_bloom;
                                }

                            } else {
                                cycle_time ct;
                                ct.start_time = timestamp;
                                ct.end_time = end_time;
                                root_candidate_approx[src][ct][dst] = old_bloom;
                            }

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
    //result.close();

    std::cout << "Time to find seeds: " << timer.LiveElapsedSeconds() << std::endl;
    std::cout << "#root founds: " << root_candidate_approx.size() << std::endl;
    std::cout << "Memory after 2nd phase: " << getMem() << std::endl;
    completeSummary.clear();
    node_update_time.clear();
    std::cout << "Memory after 2nd phase clear: " << getMem() << std::endl;
    std::cout << "*********Start compressing***********" << std::endl;
    ptime = timer.LiveElapsedSeconds();
    set<approxCandidatesNew> final_roots = compressRootCandidatesNew(&root_candidate_approx, window_bracket);
    std::cout << "Time to Compress : " << timer.LiveElapsedSeconds() - ptime << " #roots found: "
              << final_roots.size() << std::endl;
    timer.Stop();
    std::cout << "Memory after compress: " << getMem() << std::endl;
    root_candidate_approx.clear();

    std::cout << "Memory after compress clear: " << getMem() << std::endl;
    return final_roots;
}

set<approxCandidatesNew>
findRootNodesApproxBothDirectionNew(std::string input, std::string output, int window, int cleanUpLimit,
                                    bool reverseEdge) {
    map<int, bloom_filter> completeSummary;
    //map<root,map<<t_start,t_end>,<dst,approx candidateset>>>
    map<int, map<cycle_time, map<int, bloom_filter>>> root_candidate_approx;
    map<int, int> node_update_time;
    map<int, set<endNode>> rootnode_end_time_set;
    map<int, set<endNode>>::iterator rootnode_end_time_set_itr;
    bloom_parameters parameters;
    pair<int, pair<int, int>> root_neigbhour_time;
    // How many elements roughly do we expect to insert?
    parameters.projected_element_count = 5000;

    // Maximum tolerable false positive probability? (0,1)
    parameters.false_positive_probability = 0.0001; // 1 in 10000

    // Simple randomizer (optional)
    parameters.random_seed = 0xA5A5A5A5;
    parameters.compute_optimal_parameters();

    bloom_filter::optimal_param(parameters);

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

    // ofstream result;
    // result.open(output.c_str());

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
        root_neigbhour_time = updateSummary(dst, src, timestamp, window_bracket, &completeSummary, parameters,
                                            &node_update_time);
        if (root_neigbhour_time.first != 0) {
            endNode en;
            en.node_id = root_neigbhour_time.second.first;
            en.end_time = root_neigbhour_time.second.second;
            en.candidates = completeSummary[dst];
            rootnode_end_time_set[root_neigbhour_time.first].insert(en);
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
    bloom_filter old_bloom;
    cout << "Memory after first pass: " << getMem() << std::endl;
    cout<< "Candidates after first pass: "<<rootnode_end_time_set.size()<<std::endl;
    int end_neighbour;
    completeSummary.clear();
    node_update_time.clear();

    cout << "Memory after first pass after clear: " << getMem() << std::endl;
    count = 0;
    cout << rootnode_end_time_set.size() << endl;
    set<endNode>::iterator possible_end_time_set_it;

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

        root_neigbhour_time = updateSummary(src, dst, timestamp, window_bracket, &completeSummary, parameters,
                                            &node_update_time);
        if (root_neigbhour_time.first != 0) {
//check if there is a valid end for this start node.
            rootnode_end_time_set_itr = rootnode_end_time_set.find(root_neigbhour_time.first);
            if (rootnode_end_time_set_itr != rootnode_end_time_set.end()) {

                set<endNode> &possible_end_time_set = rootnode_end_time_set_itr->second;

                for (possible_end_time_set_it = possible_end_time_set.begin();
                     possible_end_time_set_it != possible_end_time_set.end(); ++possible_end_time_set_it) {
                    end_time = possible_end_time_set_it->end_time;
                    end_neighbour = possible_end_time_set_it->node_id;

                    if (end_time - timestamp > 0 & end_time - timestamp < window_bracket) {
                        if (dst != end_neighbour) {
                            //    result << src << ",";
                            //    result << timestamp << ",";//start of cycle
                            //    result << dst; //end of cycle

                            //    result << "\n";
                            old_bloom = possible_end_time_set_it->candidates;
                            old_bloom &= completeSummary[dst];
                            //create candidates for cycles
                            if (root_candidate_approx.count(src) > 0) {

                                bool added = false;
                                for (map<cycle_time, map<int, bloom_filter>>::iterator cycle_itr = root_candidate_approx[src].begin();
                                     cycle_itr != root_candidate_approx[src].end(); cycle_itr++) {
                                    if (abs(cycle_itr->first.start_time - timestamp) < window_bracket) {
                                        int min_ts = min(cycle_itr->first.start_time, timestamp);
                                        int max_te = max(cycle_itr->first.end_time, end_time);
                                        if (max_te - min_ts < window_bracket) {
                                            cycle_time old_ct = cycle_itr->first;
                                            cycle_time new_ct;
                                            new_ct.start_time = min_ts;
                                            new_ct.end_time = max_te;
                                            if (new_ct == old_ct) {

                                            } else {
                                                map<int, bloom_filter> &temp = cycle_itr->second;
                                                temp[dst] = old_bloom;
                                                root_candidate_approx[src][new_ct] = temp;
                                                root_candidate_approx[src].erase(old_ct);
                                            }
                                            added = true;
                                            break;

                                        }
                                    }
                                }
                                if (!added) {
                                    cycle_time ct;
                                    ct.start_time = timestamp;
                                    ct.end_time = end_time;
                                    root_candidate_approx[src][ct][dst] = old_bloom;
                                }

                            } else {
                                cycle_time ct;
                                ct.start_time = timestamp;
                                ct.end_time = end_time;
                                root_candidate_approx[src][ct][dst] = old_bloom;
                            }

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
    //result.close();

    std::cout << "Time to find seeds: " << timer.LiveElapsedSeconds() << std::endl;
    std::cout << "#root founds: " << root_candidate_approx.size() << std::endl;
    std::cout << "Memory after 2nd phase: " << getMem() << std::endl;
    completeSummary.clear();
    node_update_time.clear();
    std::cout << "Memory after 2nd phase clear: " << getMem() << std::endl;
    std::cout << "*********Start compressing***********" << std::endl;
    ptime = timer.LiveElapsedSeconds();
    set<approxCandidatesNew> final_roots = compressRootCandidatesNew(&root_candidate_approx, window_bracket);
    std::cout << "Time to Compress : " << timer.LiveElapsedSeconds() - ptime << " #roots found: "
              << final_roots.size() << std::endl;
    timer.Stop();
    std::cout << "Memory after compress: " << getMem() << std::endl;
    root_candidate_approx.clear();

    std::cout << "Memory after compress clear: " << getMem() << std::endl;
    return final_roots;
}
//pending
void findCyclesBothDirection(std::string input, std::string output, int window, int cleanUpLimit,
                             bool reverseEdge) {
    map<int, bloom_filter> completeSummary;
    //map<root,map<<t_start,t_end>,<dst,approx candidateset>>>
    map<int, map<cycle_time, map<int, bloom_filter>>> root_candidate_approx;
    map<int, int> node_update_time;
    map<int, set<endNode>> rootnode_end_time_set;
    map<int, set<endNode>>::iterator rootnode_end_time_set_itr;
    bloom_parameters parameters;
    pair<int, pair<int, int>> root_neigbhour_time;
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


    // ofstream result;
    // result.open(output.c_str());

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
        updateData(src, dst, timestamp);
        all_data.push_back(line);
        root_neigbhour_time = updateSummary(dst, src, timestamp, window_bracket, &completeSummary, parameters,
                                            &node_update_time);
        if (root_neigbhour_time.first != 0) {
            endNode en;
            en.node_id = root_neigbhour_time.second.first;
            en.end_time = root_neigbhour_time.second.second;
            en.candidates = completeSummary[dst];
            rootnode_end_time_set[root_neigbhour_time.first].insert(en);
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
    bloom_filter old_bloom;
    cout << "Memory after first pass: " << getMem() << std::endl;
    int end_neighbour;
    completeSummary.clear();
    node_update_time.clear();

    cout << "Memory after first pass after clear: " << getMem() << std::endl;
    count = 0;
    cout << rootnode_end_time_set.size() << endl;
    set<endNode>::iterator possible_end_time_set_it;

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

        root_neigbhour_time = updateSummary(src, dst, timestamp, window_bracket, &completeSummary, parameters,
                                            &node_update_time);
        if (root_neigbhour_time.first != 0) {
//check if there is a valid end for this start node.
            rootnode_end_time_set_itr = rootnode_end_time_set.find(root_neigbhour_time.first);
            if (rootnode_end_time_set_itr != rootnode_end_time_set.end()) {

                set<endNode> &possible_end_time_set = rootnode_end_time_set_itr->second;

                for (possible_end_time_set_it = possible_end_time_set.begin();
                     possible_end_time_set_it != possible_end_time_set.end(); ++possible_end_time_set_it) {
                    end_time = possible_end_time_set_it->end_time;
                    end_neighbour = possible_end_time_set_it->node_id;

                    if (end_time - timestamp > 0 & end_time - timestamp < window_bracket) {
                        if (dst != end_neighbour) {
                            //    result << src << ",";
                            //    result << timestamp << ",";//start of cycle
                            //    result << dst; //end of cycle

                            //    result << "\n";
                            old_bloom = possible_end_time_set_it->candidates;
                            old_bloom &= completeSummary[dst];
                            //create candidates for cycles
                            if (root_candidate_approx.count(src) > 0) {

                                bool added = false;
                                for (map<cycle_time, map<int, bloom_filter>>::iterator cycle_itr = root_candidate_approx[src].begin();
                                     cycle_itr != root_candidate_approx[src].end(); cycle_itr++) {
                                    if (abs(cycle_itr->first.start_time - timestamp) < window_bracket) {
                                        int min_ts = min(cycle_itr->first.start_time, timestamp);
                                        int max_te = max(cycle_itr->first.end_time, end_time);
                                        if (max_te - min_ts < window_bracket) {
                                            cycle_time old_ct = cycle_itr->first;
                                            cycle_time new_ct;
                                            new_ct.start_time = min_ts;
                                            new_ct.end_time = max_te;
                                            if (new_ct == old_ct) {

                                            } else {
                                                map<int, bloom_filter> &temp = cycle_itr->second;
                                                temp[dst] = old_bloom;
                                                root_candidate_approx[src][new_ct] = temp;
                                                root_candidate_approx[src].erase(old_ct);
                                            }
                                            added = true;
                                            break;

                                        }
                                    }
                                }
                                if (!added) {
                                    cycle_time ct;
                                    ct.start_time = timestamp;
                                    ct.end_time = end_time;
                                    root_candidate_approx[src][ct][dst] = old_bloom;
                                }

                            } else {
                                cycle_time ct;
                                ct.start_time = timestamp;
                                ct.end_time = end_time;
                                root_candidate_approx[src][ct][dst] = old_bloom;
                            }

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
    //result.close();

    std::cout << "Time to find Cycles: " << timer.LiveElapsedSeconds() << std::endl;

    completeSummary.clear();
    node_update_time.clear();

    timer.Stop();


}

void print(map<int, map<cycle_time, map<int, set<int>>>> root_candidate_exact) {
    int root_node;

    for (map<int, map<cycle_time, map<int, set<int>>>>::iterator iterator1 = root_candidate_exact.begin();
         iterator1 != root_candidate_exact.end(); iterator1++) {
        root_node = iterator1->first;
        if (root_node = 13140) {
            for (map<cycle_time, map<int, set<int>>>::iterator timeItr = iterator1->second.begin();
                 timeItr != iterator1->second.end(); timeItr++) {
                if (timeItr->first.start_time == 1196833689) {
                    cout << root_node << "," << timeItr->first.start_time << "," << timeItr->first.end_time << ",";
                    for (map<int, set<int>>::iterator dstIt = timeItr->second.begin();
                         dstIt != timeItr->second.end(); dstIt++) {
                        cout << dstIt->first;
                        for (int x: dstIt->second) {
                            cout << "," << x;
                        }
                    }
                    cout << endl;
                }
            }
        }
    }
}

void print(set<exactCandidates> final_roots) {
    for (auto x:final_roots) {
        if (x.root_node == 13140) {
            cout << x.root_node << "," << x.end_time << ",";
            for (auto y:x.neighbours_time) {
                cout << y.first << "-" << y.second << ",";
            }
            for (auto z:x.candidates_nodes) {
                cout << z << ",";
            }
            cout << endl;
        }
    }
}

set<exactCandidates>
findRootNodesExactBothDirection(std::string input, std::string output, int window, int cleanUpLimit,
                                bool reverseEdge) {
    map<int, set<int>> completeSummary;
    //map<root,map<<t_start,t_end>,<dst,approx candidateset>>>
    map<int, map<cycle_time, map<int, set<int>>>> root_candidate_exact;
    map<int, int> node_update_time;
    map<int, set<pair<int, int>>> rootnode_end_time_set;
    map<int, set<pair<int, int>>>::iterator rootnode_end_time_set_itr;

    pair<int, pair<int, int>> root_neigbhour_time;


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
        root_neigbhour_time = updateSummaryExact(dst, src, timestamp, window_bracket, &completeSummary,
                                                 &node_update_time);
        if (root_neigbhour_time.first != 0) {
            rootnode_end_time_set[root_neigbhour_time.first].insert(root_neigbhour_time.second);
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
    cout << "Memory after first pass: " << getMem() << std::endl;
    int end_neighbour;
    completeSummary.clear();
    node_update_time.clear();

    cout << "Memory after first pass after clear: " << getMem() << std::endl;
    count = 0;
    cout << rootnode_end_time_set.size() << endl;
    set<pair<int, int>>::iterator possible_end_time_set_it;

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

        root_neigbhour_time = updateSummaryExact(src, dst, timestamp, window_bracket, &completeSummary,
                                                 &node_update_time);
        if (root_neigbhour_time.first != 0) {
//check if there is a valid end for this start node.
            rootnode_end_time_set_itr = rootnode_end_time_set.find(root_neigbhour_time.first);
            if (rootnode_end_time_set_itr != rootnode_end_time_set.end()) {

                set<pair<int, int>> &possible_end_time_set = rootnode_end_time_set_itr->second;

                for (possible_end_time_set_it = possible_end_time_set.begin();
                     possible_end_time_set_it != possible_end_time_set.end(); ++possible_end_time_set_it) {
                    end_time = possible_end_time_set_it->second;
                    end_neighbour = possible_end_time_set_it->first;
                    if (end_time - timestamp > 0 & end_time - timestamp < window_bracket) {
                        if (dst != end_neighbour) {
                            result << src << ",";
                            result << timestamp << ",";//start of cycle
                            result << dst; //end of cycle

                            result << "\n";
                            //create candidates for cycles
                            cycle_time ct;
                            ct.start_time = timestamp;
                            ct.end_time = end_time;
                            root_candidate_exact[src][ct][dst] = completeSummary[dst];
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
    result.close();

    std::cout << "Time to find seeds: " << timer.LiveElapsedSeconds() << std::endl;
    std::cout << "#root founds: " << root_candidate_exact.size() << std::endl;
    std::cout << "Memory after 2nd phase: " << getMem() << std::endl;
    completeSummary.clear();
    node_update_time.clear();
    std::cout << "Memory after 2nd phase clear: " << getMem() << std::endl;
    std::cout << "*********Start compressing***********" << std::endl;
    ptime = timer.LiveElapsedSeconds();
    set<exactCandidates> final_roots = compressRootCandidates(&root_candidate_exact, window_bracket);
    std::cout << "Time to Compress : " << timer.LiveElapsedSeconds() - ptime << " #roots found: "
              << final_roots.size() << std::endl;
    timer.Stop();
    std::cout << "Memory after compress: " << getMem() << std::endl;
    root_candidate_exact.clear();

    std::cout << "Memory after compress clear: " << getMem() << std::endl;
    return final_roots;
    //print(final_roots);


}

set<approxCandidatesNew>
compressRootCandidatesNew(map<int, map<cycle_time, map<int, bloom_filter>>> *root_candidates,
                          int window_bracket) {
    set<approxCandidatesNew> result;
    int root_node, max_end_time;
    int count = 0;
    map<int, map<cycle_time, map<int, bloom_filter>>> &root_candidates_approx = *root_candidates;

    for (map<int, map<cycle_time, map<int, bloom_filter>>>::iterator it_root = root_candidates_approx.begin();
         it_root != root_candidates_approx.end(); ++it_root) {
        root_node = it_root->first;

        approxCandidatesNew *ac;
        for (map<cycle_time, map<int, bloom_filter>>::iterator it_inner = it_root->second.begin();
             it_inner != it_root->second.end(); ++it_inner) {
            if (count == 0) {
                //create new candidates
                ac = new approxCandidatesNew();
                ac->start_time = it_inner->first.start_time;
                max_end_time = it_inner->first.start_time + window_bracket;
                //update candidates
                ac->root_node = root_node;
                ac->end_time = it_inner->first.end_time;
                ac->neighbours_candidates = it_inner->second;

            } else {
                if (it_inner->first.end_time < max_end_time) {
                    //update candidates
                    if (it_inner->first.end_time > ac->end_time) {
                        ac->end_time = it_inner->first.end_time;
                    }
                    for (map<int, bloom_filter>::iterator neighbours_itr = it_inner->second.begin();
                         neighbours_itr != it_inner->second.end(); neighbours_itr++) {
                        if (ac->neighbours_candidates.count(neighbours_itr->first) > 0) {
                            ac->neighbours_candidates[neighbours_itr->first] |= neighbours_itr->second;
                        } else {
                            ac->neighbours_candidates[neighbours_itr->first] = neighbours_itr->second;
                        }
                    }

                } else {
                    //add the old candidate in result
                    result.insert(*ac);
                    //create new candidates
                    ac = new approxCandidatesNew();
                    ac->start_time = it_inner->first.start_time;
                    max_end_time = it_inner->first.start_time + window_bracket;
                    //update candidates
                    ac->end_time = it_inner->first.end_time;
                    ac->root_node = root_node;
                    ac->neighbours_candidates = it_inner->second;

                }
            }
            count++;
        }
        if (count > 0) {
            result.insert(*ac);
            count = 0;
        }
    }

    return result;
}

set<exactCandidates>
compressRootCandidates(map<int, map<cycle_time, map<int, set<int>>>> *root_candidates,
                       int window_bracket) {
    set<exactCandidates> result;
    int root_node, max_end_time;
    int count = 0;
    map<int, map<cycle_time, map<int, set<int>>>> &root_candidates_approx = *root_candidates;

    for (map<int, map<cycle_time, map<int, set<int>>>>::iterator it_root = root_candidates_approx.begin();
         it_root != root_candidates_approx.end(); ++it_root) {
        root_node = it_root->first;
        exactCandidates *ac;
        for (map<cycle_time, map<int, set<int>>>::iterator it_inner = it_root->second.begin();
             it_inner != it_root->second.end(); ++it_inner) {
            if (count == 0) {
                //create new candidates
                ac = new exactCandidates();
                max_end_time = it_inner->first.start_time + window_bracket;
                //update candidates
                ac->root_node = root_node;
                ac->end_time = it_inner->first.end_time;
                mergeSummaries(&(it_inner->second), ac, it_inner->first.start_time);

            } else {
                if (it_inner->first.end_time < max_end_time) {
                    //update candidates
                    if (it_inner->first.end_time > ac->end_time) {
                        ac->end_time = it_inner->first.end_time;
                    }
                    mergeSummaries(&(it_inner->second), ac, it_inner->first.start_time);
                } else {
                    //add the old candidate in result
                    result.insert(*ac);
                    //create new candidates
                    ac = new exactCandidates();
                    max_end_time = it_inner->first.start_time + window_bracket;
                    //update candidates
                    ac->end_time = it_inner->first.end_time;
                    ac->root_node = root_node;
                    mergeSummaries(&(it_inner->second), ac, it_inner->first.start_time);

                }
            }
            count++;
        }
        if (count > 0) {
            result.insert(*ac);
            count = 0;
        }
    }

    return result;
}

void mergeSummaries(map<int, set<int>> *summary, exactCandidates *ac, int start_time) {
    bool first_neighbour = true;
    if (ac->neighbours_time.size() > 0) {
        first_neighbour = false;
    }
    map<int, set<int>> &set_summary = *summary;
    for (map<int, set<int>>::iterator it_neighbours = set_summary.begin();
         it_neighbours != set_summary.end(); ++it_neighbours) {
        ac->neighbours_time.insert(make_pair(it_neighbours->first, start_time));
        if (first_neighbour) {
            ac->candidates_nodes = it_neighbours->second;
            first_neighbour = false;
        } else {
            ac->candidates_nodes.insert(it_neighbours->second.begin(), it_neighbours->second.end());
        }
    }
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
/*
        if (completeSummary.count(src) == 0) {
            //Instantiate Bloom Filter
            bloom_filter filter(parameters);
             filter.insert(dst);
            completeSummary[src] = filter;

        } else {
            completeSummary[src].insert(dst);

        }
        */
        completeSummary[src].insert(dst);
        node_update_time[src] = timestamp;
        src_iterator = completeSummary.find(src);
        dst_iterator = completeSummary.find(dst);
        bloom_filter &src_summary = src_iterator->second;

        //if src summary exist transfer it to dst  if it is in window prune away whats not in window
        if (dst_iterator != completeSummary.end() & node_update_time.count(dst) > 0) {
            if (abs(node_update_time[dst] - timestamp) > window_bracket) {
                dst_iterator->second.clean();
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

pair<int, pair<int, int>>
updateSummaryExact(int src, int dst, int timestamp, int window_bracket, map<int, set<int>> *summary,
                   map<int, int> *update_time) {
    map<int, set<int>>::iterator src_iterator;
    map<int, set<int>>::iterator dst_iterator;
    map<int, set<int>> &completeSummary = *summary;
    map<int, int> &node_update_time = *update_time;
    pair<int, pair<int, int>> result;

    if (src == dst) {
        //self loop ignored
    } else {

        if (completeSummary.count(src) == 0) {
            //Instantiate Bloom Filter
            set<int> filter;
            filter.insert(dst);
            completeSummary[src] = filter;

        } else {
            completeSummary[src].insert(dst);

        }
        node_update_time[src] = timestamp;
        src_iterator = completeSummary.find(src);
        dst_iterator = completeSummary.find(dst);
        set<int> &src_summary = src_iterator->second;

        //if src summary exist transfer it to dst  if it is in window prune away whats not in window
        if (dst_iterator != completeSummary.end() & node_update_time.count(dst) > 0) {
            if (abs(node_update_time[dst] - timestamp) > window_bracket) {
                dst_iterator->second.clear();
            } else {
                if (dst_iterator->second.count(src) > 0) {

                    if (dst_iterator->second.size() > 1) {
                        result = make_pair(src, make_pair(dst, timestamp));

                    }

                }
                src_summary.insert(dst_iterator->second.begin(), dst_iterator->second.end());

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

int
cleanup(map<int, set<int>> *completeSummary, map<int, int> *node_update_time, int timestamp, int window_bracket) {
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


string combineSeeds(std::string root_file, int window) {
    string output_file = root_file;
    output_file.replace(output_file.end() - 4, output_file.end(), "-compressed.csv");
    ofstream result;
    result.open(output_file.c_str());
    int window_bracket = window * 60 * 60;
    ifstream root_file_data(root_file.c_str());
    string line;
    std::vector<std::string> templine;
    int root_node, t_s, t_e;
    set<int> candidates;
    map<int, vector<seed>> all_seeds;

    while (root_file_data >> line) {
        templine = Tools::Split(line, ',');
        root_node = stoi(templine[0]);
        t_s = stoi(templine[1]);
        t_e = stoi(templine[2]);
        candidates.clear();
        for (int i = 3; i < templine.size(); i++) {
            candidates.insert(stoi(templine[i]));
        }
        seed s;
        s.start_time = t_s;
        s.end_time = t_e;
        s.candidates = candidates;
        all_seeds[root_node].push_back(s);
    }

    int max_end_time, current_start_time, current_end_time;
    for (map<int, vector<seed>>::iterator seed_itr = all_seeds.begin();
         seed_itr != all_seeds.end(); seed_itr++) {
        root_node = seed_itr->first;
        sort(seed_itr->second.begin(), seed_itr->second.end());
        seed current_seed = seed_itr->second[0];
        current_start_time = current_seed.start_time;
        current_end_time = current_seed.end_time;
        max_end_time = current_start_time + window_bracket;
        for (int i = 1; i < seed_itr->second.size(); i++) {
            current_end_time = seed_itr->second[i].end_time;
            if (current_end_time < max_end_time) {
                if (current_seed.end_time < current_end_time) {
                    current_seed.end_time = current_end_time;
                }
                current_seed.candidates.insert(seed_itr->second[i].candidates.begin(),
                                               seed_itr->second[i].candidates.end());
            } else {
                result << root_node << "," << current_seed.pringString() << endl;
                current_seed = seed_itr->second[i];
                current_start_time = current_seed.start_time;
                current_end_time = current_seed.end_time;
                max_end_time = current_start_time + window_bracket;
            }

        }
        result << root_node << "," << current_seed.pringString() << endl;
    }
    result.close();

    return output_file;
}

