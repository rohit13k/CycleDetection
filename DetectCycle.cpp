//
// Created by Rohit on 21-Jun-17.
//

#include "DetectCycle.h"
#include "Split.h"
#include "Timer.h"
#include "MemoryMonitor.h"
#include <fstream>
#include <stack>
#include <queue>

using namespace std;

map<nodeid, timeGroup> rootNodes;
map<nodeid, int> ct;//closing times
std::set<string> resultAllPath;
std::map<int, map<int, vector<pathBundle>>> resultAllPathBundle;
int cycle_count = 0;

map<nodeid, set<pair<nodeid, int>>> U;//unblock list
int max_E_count;

int findRootNodesAdv(std::string input, std::string output, int window, int cleanUpLimit,
                     bool reverseEdge) {

    double_llist last_updated_time_list;
    map<nodeid, nodeSummary> completeSummaryAdvanced;
    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    int src, dst;
    int negativeTimestamp;
    string line;
    Platform::Timer timer;
    timer.Start();
    int timestamp;
    int temptime = 0;
    int count = 0;
    bool cycleFound = false;
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string tempnode = "";
    ofstream result;
    result.open(output.c_str());
    node *position_in_time_list;


    while (infile >> line) {
        try {
            templine = Tools::Split(line, ',');
            src = stoi(templine[0]);
            dst = stoi(templine[1]);
            if (reverseEdge) {
                src = stoi(templine[1]);
                dst = stoi(templine[0]);
            }
            timestamp = stol(templine[2].c_str());

            if (src == dst) {
                //self loop ignored
            } else {

                //add src in the destination summary
                completeSummaryAdvanced[dst].summary[-1 * timestamp].insert(src);
                //update the position of dst node in time list
                position_in_time_list = last_updated_time_list.add_begin(timestamp, dst);
                if (completeSummaryAdvanced[dst].position_in_time_list != NULL) {
                    cout << "before delete" << endl;
                    last_updated_time_list.display_dlist();
                    last_updated_time_list.delete_element(completeSummaryAdvanced[dst].position_in_time_list);
                    cout << "after delete" << endl;
                    last_updated_time_list.display_dlist();
                }
                //save the position of the dst node in time list in the summary
                completeSummaryAdvanced[dst].position_in_time_list = position_in_time_list;
                //if src summary exist transfer it to dst  if it is in window prune away whats not in window
                if (completeSummaryAdvanced.count(src) > 0) {
                    for (map<int, set<nodeid>>::iterator it = completeSummaryAdvanced[src].summary.begin();
                         it != completeSummaryAdvanced[src].summary.end(); ++it) {
                        if ((-1 * it->first) > timestamp - window_bracket) {

                            if (it->second.count(dst) > 0) {
                                //the destination is already in src summary hence a cycle exist
                                set<nodeid> candidates = getCandidates(completeSummaryAdvanced[src].summary,
                                                                       -1 * it->first,
                                                                       (-1 * it->first) + window_bracket);
                                candidates.erase(dst);
                                candidates.insert(src);
                                if (candidates.size() > 1) {//only cycles having more than 1 nodes

                                    result << dst << ",";
                                    result << (-1 * it->first) << ",";//start of cycle
                                    result << timestamp << ","; //end of cycle
                                    for (int x:candidates) {
                                        result << x << ",";

                                    }
                                    result << "\n";

                                }
                                // add other in the summary
                                for (auto x:it->second) {
                                    if (x != dst) {
                                        completeSummaryAdvanced[dst].summary[it->first].insert(x);
                                    }
                                }
                            } else {
                                completeSummaryAdvanced[dst].summary[it->first].insert(it->second.begin(),
                                                                                       it->second.end());
                            }
                        } else {
                            completeSummaryAdvanced[src].summary.erase(it, completeSummaryAdvanced[src].summary.end());
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

                std::cout << "finished parsing, count," << count << "," << parseTime << "," << getMem();
                cout << ",summary size," << completeSummaryAdvanced.size() << " delete count,";
                cout << cleanupAdv(timestamp, window_bracket, &last_updated_time_list);
                std::cout << " ,clean time," << timer.LiveElapsedSeconds() - ptime << std::endl;
            }

        } catch (const std::exception &e) {
            std::cout << "Caught exception \"" << e.what() << "\"\n";
        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    timer.Stop();

    result.close();
    return 0;
}

int cleanupAdv(int timestamp, int window_bracket, double_llist *last_updated_time_list) {

    int time = timestamp + window_bracket + 1;
    vector<int> deletelist = last_updated_time_list->get_expired_nodes(time);
    for (auto x: deletelist) {
        //  completeSummaryAdvanced.erase(x);
    }
    //  last_updated_time_list.delete_expired(timestamp + window_bracket + 1);

    return deletelist.size();

}


int findRootNodes(std::string input, std::string output, int window, int cleanUpLimit,
                  bool reverseEdge) {
    map<nodeid, map<int, set<nodeid>>> completeSummary;


    std::vector<std::string> templine;
    ifstream infile(input.c_str());
    int src, dst;
    int negativeTimestamp;
    string line;
    Platform::Timer timer;
    timer.Start();
    int timestamp;
    int temptime = 0l;
    int count = 0;
    bool cycleFound = false;
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string tempnode = "";
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
        timestamp = stol(templine[2].substr(0, 10).c_str());

        if (src != dst) {
            //self loop ignored
        } else {

            //add src in the destination summary
            completeSummary[dst][-1 * timestamp].insert(src);

            //if src summary exist transfer it to dst  if it is in window prune away whats not in window
            if (completeSummary.count(src) > 0) {
                for (map<int, set<nodeid>>::iterator it = completeSummary[src].begin();
                     it != completeSummary[src].end(); ++it) {
                    if ((-1 * it->first) > timestamp - window_bracket) {

                        if (it->second.count(dst) > 0) {
                            //the destination is already in src summary hence a cycle exist
                            set<nodeid> candidates = getCandidates(completeSummary[src], -1 * it->first,
                                                                   (-1 * it->first) + window_bracket);
                            candidates.erase(dst);
                            candidates.insert(src);
                            if (candidates.size() > 1) {//only cycles having more than 1 nodes

                                result << dst << ",";
                                result << (-1 * it->first) << ",";//start of cycle
                                result << timestamp << ","; //end of cycle
                                for (int x:candidates) {
                                    result << x << ",";

                                }
                                result << "\n";

                            }
                            // add other in the summary
                            for (auto x:it->second) {
                                if (x != dst) {
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
                }//for loop
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

int cleanup(map<nodeid, map<int, set<nodeid>>> *completeSummary, int timestamp, int window_bracket) {
    int size = 0;
    int src;
    vector<int> deletelist;
    for (map<nodeid, map<int, set<nodeid>>>::iterator it = completeSummary->begin();
         it != completeSummary->end(); ++it) {

        size = it->second.size();
        src = it->first;
        if (size > 0) {
            for (map<int, set<nodeid>>::iterator itinner = it->second.begin();
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

set<nodeid> getCandidates(map<int, set<nodeid>> summary, int t_s, int t_e) {
    set<nodeid> candidates;
    int time;
    for (map<int, set<nodeid>>::iterator it = summary.begin(); it != summary.end(); ++it) {
        time = -1 * it->first;
        if (time >= t_s && time < t_e) {

            candidates.insert(it->second.begin(), it->second.end());
        }
    }
    return candidates;
}

int findAllCycle(std::string dataFile, std::string rootNodeFile, std::string output, int window,
                 bool isCompressed, bool reverseEdge, bool candidates_provided, bool use_bundle) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    string monitor_result = dataFile;
    std::string ext;
    ext = "-monitor-" + to_string(window);
    if (use_bundle) {
        ext = ext + "-bundle";
    }
    ext = ext + ".csv";
    monitor_result.replace(monitor_result.end() - 4, monitor_result.end(), ext);
    ofstream monitor_writer;
    monitor_writer.open(monitor_result.c_str());
    monitor_writer << "root,neigbhour,candidate_count,edge_count,time,cycles\n";
    ofstream cycleResult;
    cycleResult.open(output.c_str());
    string line;
    string monitor_text;
    Platform::Timer timer;

    max_E_count = 0;

    timer.Start();
    readFile(dataFile, reverseEdge);//creates a data structure of type <srcNode:<time:dstNode>>
    ptime = timer.LiveElapsedSeconds();

    std::cout << "finished reading " << ptime
              << std::endl;

    std::vector<std::string> templine;
    ifstream infile(rootNodeFile.c_str());
    nodeid rootnode;
    int t_s;
    int t_end;
    int i = 0;
    int count = 0;
    vector<int> all_cycle(50);
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        set<nodeid> candidateset;
        rootnode = stoi(templine[0]);
        t_s = stol(templine[1].c_str());
        t_end = stol(templine[2].c_str());
        for (i = 3; i < templine.size(); i++) {
            candidateset.insert(stoi(templine[i]));
        }
        candidateset.erase(rootnode);
        // findCycle(rootnode, t_s, &candidateset, window_bracket);
        if (candidateset.size() > 1) {
            candidateset.insert(rootnode);
            //run only for cycle with lenght greater than 2
            // monitor.Start();
            //  monitorTime = time(&now);

            monitor_text = DynamicDFS(rootnode, t_s, t_end + 1, candidateset, window_bracket, isCompressed,
                                      candidates_provided, use_bundle, &all_cycle);
            monitor_writer << monitor_text;
            /*
            cout << "Monitor Time," << monitor.LiveElapsedSeconds() << ",rootnode, ";
            cout << rootnode << ",start time," << t_s << ",candidate set size,";
            cout << candidateset.size() << ",#cycles," << cyclesfound.size();
            for (auto c:cyclesfound) {
                cout << "," << c;
            }
            cout << endl;
            */
            //  monitor.Stop();
        }
        count++;
        if (count % 1000 == 0) {
            //do cleanup

            double parseTime = timer.LiveElapsedSeconds() - ptime;
            ptime = timer.LiveElapsedSeconds();

            std::cout << "finished cycle roots, count," << count << ",time," << parseTime;
            cout << ",cycle found," << resultAllPath.size() << ",max E: " << max_E_count;
            cout << ",memory," << getMem() << std::endl;
        }

    }

    monitor_writer.close();
    int root_node;
    if (use_bundle) {
        //  map<int, int> cycle_3_count;
        int cycle_length, cycle_count;
        int maxCycleLenght = 0;
        for (map<int, map<int, vector<pathBundle>>>::iterator paths_itr = resultAllPathBundle.begin();
             paths_itr != resultAllPathBundle.end(); paths_itr++) {
            root_node = paths_itr->first;
            //   cycle_3_count[root_node] = 0;
            for (map<int, vector<pathBundle>>::iterator it_inner = paths_itr->second.begin();
                 it_inner != paths_itr->second.end(); it_inner++) {
                cycle_length = it_inner->first;
                if (cycle_length > maxCycleLenght) {
                    maxCycleLenght = cycle_length;
                }
                pathBundle lastBundle, currentBundle;
                int count = 0;
                vector<pathBundle> &all_path = it_inner->second;
                sort(all_path.begin(), all_path.end());
                for (vector<pathBundle>::iterator it = all_path.begin(); it != all_path.end(); it++) {
                    currentBundle = *it;
                    if (count == 0) {
                        lastBundle = currentBundle;
                        cycle_count = pathCount(currentBundle);
                        //  if (cycle_length == 3) {
                        //       cycle_3_count[root_node] = cycle_3_count[root_node] + cycle_count;
                        //   }
                        cycleResult << currentBundle.printPath() << "\n";

                        all_cycle[cycle_length] += cycle_count;
                    } else {
                        //check if its overlaping to last one
                        if (is_overlapping(&currentBundle, &lastBundle)) {


                            for (auto x:lastBundle.path[cycle_length - 1].time.times) {
                                currentBundle.path[cycle_length - 1].time.times.erase(x);
                            }
                            if (currentBundle.path[cycle_length - 1].time.size() > 0) {
                                cycle_count = pathCount(currentBundle);
                                //      if (cycle_length == 3) {
                                //          cycle_3_count[root_node] = cycle_3_count[root_node] + cycle_count;
                                //       }
                                cycleResult << currentBundle.printPath() << "\n";
                                all_cycle[cycle_length] += cycle_count;


                                for (auto x:currentBundle.path[cycle_length - 1].time.times) {
                                    lastBundle.path[cycle_length - 1].time.times.insert(x);
                                }
                                for (auto x:currentBundle.path[0].time.times) {
                                    lastBundle.path[0].time.times.insert(x);
                                }

                            }


                        } else {
                            lastBundle = currentBundle;
                            cycle_count = pathCount(currentBundle);
                            //   if (cycle_length == 3) {
                            //      cycle_3_count[root_node] = cycle_3_count[root_node] + cycle_count;
                            //  }
                            cycleResult << currentBundle.printPath() << "\n";
                            all_cycle[cycle_length] += cycle_count;
                        }
                    }
                    count++;

                }
            }
        }

        for (int i = 1; i <= maxCycleLenght; i++) {
            cout << i << "," << all_cycle[i] << endl;
        }

/*
        cout << "printing count for length 3" << endl;
        for (map<int, int>::iterator it = cycle_3_count.begin(); it != cycle_3_count.end(); it++) {
            if (it->second != 0) {
                cout << it->first << "," << it->second << endl;
            }
        }
*/

    } else {
        int cycleLengthArray[50] = {0};
        int cycleLenght;
        int maxCycleLenght = 0;
        for (auto x:resultAllPath) {
            templine = Tools::Split(x, ',');
            cycleLenght = stoi(templine[1]);

            if (cycleLenght < 50) {
                cycleLengthArray[cycleLenght]++;
                if (cycleLenght > maxCycleLenght) {
                    maxCycleLenght = cycleLenght;
                }
            } else
                cout << "cycle of length greather than 50 found";
            if (output.compare("") != 0) {
                cycleResult << x << "\n";
            }
            //cout << x << endl;
        }
        cout << "cycles:" << endl;
        for (int i = 1; i <= maxCycleLenght; i++) {
            cout << i << "," << cycleLengthArray[i] << endl;
        }

    }

}

bool is_overlapping(pathBundle *pathBundle1, pathBundle *pathBundle2) {
    bool result = false;
    pathBundle &bundle1 = *pathBundle1;
    pathBundle &bundle2 = *pathBundle2;
    std::vector<int> v_intersection;


    if (bundle1.getRootNode() == bundle2.getRootNode()) {
        if (bundle1.path.size() == bundle2.path.size()) {
            v_intersection.clear();
            std::set_intersection(bundle1.path[0].time.times.begin(), bundle1.path[0].time.times.end(),
                                  bundle2.path[0].time.times.begin(), bundle2.path[0].time.times.end(),
                                  std::back_inserter(v_intersection));
            if (v_intersection.size() > 0) {
                for (int i = 0; i < bundle1.path.size() - 1; i++) {
                    if (bundle1.path[i].to_node == bundle2.path[i].to_node) {
                        result = true;
                    } else {
                        result = false;
                        break;
                    }
                }

            }

        }
    }


    return result;
}


int findAllCycleUsingBloom(std::string dataFile, set<approxCandidatesNew> *root_candidates, std::string output,
                           int window, bool reverseEdge, bool use_bundle) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    set<approxCandidatesNew> &root_candidate_approx = *root_candidates;
    set<approxCandidatesNew>::iterator root_candidate_approx_itr;
    time_t now;
    max_E_count = 0;
    ofstream cycleResult;
    cycleResult.open(output.c_str());
    string line;
    Platform::Timer timer;
    timer.Start();
    readFile(dataFile, reverseEdge);//creates a data structure of type <srcNode:<time:dstNode>>
    ptime = timer.LiveElapsedSeconds();

    std::cout << "finished reading " << ptime << std::endl;

    vector<int> all_cycle(50);
    int t_s;
    int t_end;
    int count = 0;
    int rootnode;
    int root_neigbour;
    // bloom_filter candidateset;
    for (root_candidate_approx_itr = root_candidate_approx.begin();
         root_candidate_approx_itr != root_candidate_approx.end(); ++root_candidate_approx_itr) {
        rootnode = root_candidate_approx_itr->root_node;
        cout<<"finding cycle for "<<rootnode<<" neibhours: "<<root_candidate_approx_itr->neighbours_candidates.size()<<endl;
        DynamicDFSApprox(*root_candidate_approx_itr, window_bracket, use_bundle, &all_cycle);
        count++;
        if (count % 100 == 0) {
            cout << "finished processing, " << count << " memory, " << getMem() << " cycle found, "
                 << resultAllPath.size() << ",max E: " << max_E_count << endl;
        }
    }
    cout << "finished processing, " << count << " memory, " << getMem() << " cycle found, " << cycle_count
         << ",max E: " << max_E_count << endl;
    int root_node;
    if (use_bundle) {
        //  map<int, int> cycle_3_count;
        int cycle_length, cycle_count;
        int maxCycleLenght = 0;
        for (map<int, map<int, vector<pathBundle>>>::iterator paths_itr = resultAllPathBundle.begin();
             paths_itr != resultAllPathBundle.end(); paths_itr++) {
            root_node = paths_itr->first;
            //  cycle_3_count[root_node] = 0;
            for (map<int, vector<pathBundle>>::iterator it_inner = paths_itr->second.begin();
                 it_inner != paths_itr->second.end(); it_inner++) {
                cycle_length = it_inner->first;
                if (cycle_length > maxCycleLenght) {
                    maxCycleLenght = cycle_length;
                }
                pathBundle lastBundle, currentBundle;
                int count = 0;
                vector<pathBundle> &all_path = it_inner->second;
                sort(all_path.begin(), all_path.end());
                for (vector<pathBundle>::iterator it = all_path.begin(); it != all_path.end(); it++) {
                    currentBundle = *it;
                    if (count == 0) {
                        lastBundle = currentBundle;
                        cycle_count = pathCount(currentBundle);
                        //  if (cycle_length == 3) {
                        //      cycle_3_count[root_node] = cycle_3_count[root_node] + cycle_count;
                        //  }
                        cycleResult << currentBundle.printPath() << "\n";

                        all_cycle[cycle_length] += cycle_count;
                    } else {
                        //check if its overlaping to last one
                        if (is_overlapping(&currentBundle, &lastBundle)) {


                            for (auto x:lastBundle.path[cycle_length - 1].time.times) {
                                currentBundle.path[cycle_length - 1].time.times.erase(x);
                            }
                            if (currentBundle.path[cycle_length - 1].time.size() > 0) {
                                cycle_count = pathCount(currentBundle);
                                //       if (cycle_length == 3) {
                                //           cycle_3_count[root_node] = cycle_3_count[root_node] + cycle_count;
                                //       }
                                cycleResult << currentBundle.printPath() << "\n";
                                all_cycle[cycle_length] += cycle_count;


                                for (auto x:currentBundle.path[cycle_length - 1].time.times) {
                                    lastBundle.path[cycle_length - 1].time.times.insert(x);
                                }
                                for (auto x:currentBundle.path[0].time.times) {
                                    lastBundle.path[0].time.times.insert(x);
                                }

                            }


                        } else {
                            lastBundle = currentBundle;
                            cycle_count = pathCount(currentBundle);
                            //  if (cycle_length == 3) {
                            //      cycle_3_count[root_node] = cycle_3_count[root_node] + cycle_count;
                            //  }
                            cycleResult << currentBundle.printPath() << "\n";
                            all_cycle[cycle_length] += cycle_count;
                        }
                    }
                    count++;

                }
            }
        }

        for (int i = 1; i <= maxCycleLenght; i++) {
            cout << i << "," << all_cycle[i] << endl;
        }
/*
        cout << "printing count for length 3" << endl;
        for (map<int, int>::iterator it = cycle_3_count.begin(); it != cycle_3_count.end(); it++) {
            if (it->second != 0) {
                cout << it->first << "," << it->second << endl;
            }
        }
*/

    } else {
        int cycleLengthArray[50] = {0};
        int cycleLenght;
        int maxCycleLenght = 0;

        std::vector<std::string> templine;
        for (auto x:resultAllPath) {
            templine = Tools::Split(x, ',');
            cycleLenght = stoi(templine[1]);

            if (cycleLenght < 50) {
                cycleLengthArray[cycleLenght]++;
                if (cycleLenght > maxCycleLenght) {
                    maxCycleLenght = cycleLenght;
                }
            } else
                cout << "cycle of length greather than 50 found";
            if (output.compare("") != 0) {
                cycleResult << x << "\n";
            }
            //cout << x << endl;
        }
        cout << "cycles:" << endl;
        for (int i = 1; i <= maxCycleLenght; i++) {
            cout << i << "," << cycleLengthArray[i] << endl;
        }
    }

}


int findAllCycleUsingSet(std::string dataFile, set<exactCandidates> *root_candidates, std::string output,
                         int window, bool reverseEdge, bool use_bundle) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    set<exactCandidates> &root_candidate_approx = *root_candidates;
    set<exactCandidates>::iterator root_candidate_approx_itr;
    time_t now;

    ofstream cycleResult;
    cycleResult.open(output.c_str());
    string line;
    Platform::Timer timer;
    timer.Start();
    readFile(dataFile, reverseEdge);//creates a data structure of type <srcNode:<time:dstNode>>
    ptime = timer.LiveElapsedSeconds();
    vector<int> all_cycle(50);
    std::cout << "finished reading " << ptime << std::endl;


    int t_s;
    int t_end;
    int count = 0;
    int rootnode;
    set<int> candidateset;
    for (root_candidate_approx_itr = root_candidate_approx.begin();
         root_candidate_approx_itr != root_candidate_approx.end(); ++root_candidate_approx_itr) {
        DynamicDFSExact(*root_candidate_approx_itr, window_bracket, use_bundle, &all_cycle);
        count++;
        if (count % 1000 == 0) {
            cout << "finished processing, " << count << " memory, " << getMem() << " cycle found, "
                 << resultAllPath.size() << endl;
        }

    }
    cout << "finished processing, " << count << " memory, " << getMem() << " cycle found, " << resultAllPath.size()
         << endl;
    if (use_bundle) {
        for (int i = 1; i < all_cycle.size() - 1; i++) {
            cout << i << "," << all_cycle[i] << endl;
        }
        for (auto x:resultAllPath) {
            cycleResult << x << "\n";

        }
    } else {
        int cycleLengthArray[50] = {0};
        int cycleLenght;
        int maxCycleLenght = 0;

        std::vector<std::string> templine;
        for (auto x:resultAllPath) {
            templine = Tools::Split(x, ',');
            cycleLenght = stoi(templine[1]);

            if (cycleLenght < 50) {
                cycleLengthArray[cycleLenght]++;
                if (cycleLenght > maxCycleLenght) {
                    maxCycleLenght = cycleLenght;
                }
            } else
                cout << "cycle of length greather than 50 found";
            if (output.compare("") != 0) {
                cycleResult << x << "\n";
            }
            //cout << x << endl;
        }
        cout << "cycles:" << endl;
        for (int i = 1; i <= maxCycleLenght; i++) {
            cout << i << "," << cycleLengthArray[i] << endl;
        }
    }
}

void findCycle(nodeid rootNode, int t_s, std::set<nodeid> *candidates, int window_bracket) {
    set<pedge> neighbours = getFilteredData(rootNode, t_s);
    vector<std::string> initialpath;
    set<nodeid> seen;
    for (set<pedge>::iterator eit = neighbours.begin(); eit != neighbours.end(); ++eit) {
        initialpath.clear();
        seen.clear();
        initialpath.push_back(to_string(rootNode) + ":");
        initialpath.push_back(to_string(eit->time) + "," + to_string(eit->toVertex));
        seen.insert(eit->toVertex);
        if (findTemporalPath(eit->toVertex, rootNode, t_s, t_s + window_bracket, &initialpath, seen, candidates)) {
            std::cout << "path: " << initialpath.size() << " : ";
            for (int i = 0; i < initialpath.size(); i++) {
                std::cout << "->" << initialpath[i];
            }
            cout << endl;
        }
    }

}

bool findTemporalPath(int src, int dst, int t_s, int t_end, vector<std::string> *path_till_here,
                      set<int> seen, std::set<int> *candidates) {

    vector<string> path_till_here_bkp;
    for (int i = 0; i < path_till_here->size(); i++) {
        path_till_here_bkp.push_back((*path_till_here)[i]);
    }

    set<pedge> Y = getFilteredData(src, t_s, t_end, candidates);
    bool found = false;
    for (set<pedge>::iterator edgeIt = Y.begin(); edgeIt != Y.end(); ++edgeIt) {
        if (edgeIt->toVertex == dst) {
            path_till_here->push_back(to_string(edgeIt->time) + "," + to_string(edgeIt->toVertex));
            return true;
        } else if (seen.count(edgeIt->toVertex) == 0) {
            seen.insert(edgeIt->toVertex);
            path_till_here->push_back(to_string(edgeIt->time) + "," + to_string(edgeIt->toVertex));
            found = findTemporalPath(edgeIt->toVertex, dst, edgeIt->time + 1, t_end, path_till_here, seen, candidates);

            if (found) {
                //   std::cout << "Path: " << path_till_here->size() << " : ";
                for (int i = 0; i < path_till_here->size(); i++) {
                    //     std::cout << "->" << (*path_till_here)[i];

                }
                path_till_here->clear();
                for (int i = 0; i < path_till_here_bkp.size(); i++) {
                    path_till_here->push_back(path_till_here_bkp[i]);
                }
                cout << endl;
                found = false;
            } else {
                return found;
            }

        }
    }
    return found;

}

void unblock(nodeid v, int t_v, int t_e) {
    if (ct.count(v) > 0) {
        if (t_v > ct[v]) {
            ct[v] = t_v;
            if (U.count(v) > 0) {
                set<pair<nodeid, int>> newV;
                for (set<pair<nodeid, int>>::iterator it = U[v].begin(); it != U[v].end(); ++it) {
                    int timew = it->second;
                    if (timew < t_v) {
                        int t_max = getMaxTime(it->first, v, t_v);
                        int nodew = it->first;
                        unblock(nodew, t_max, t_e);

                        //   U[v].erase(make_pair(nodew,timew));
                        int t_min = getMinTime(nodew, v, t_v, t_e);
                        if (t_min != std::numeric_limits<int>::max()) {
                            newV.insert(std::make_pair(nodew, t_min));

                        }
                    } else {
                        newV.insert(*it);
                    }
                }
                U[v].clear();
                U[v] = newV;
            }
        }
    }
}


bool
allPath(nodeid w, nodeid rootnode, int t_s, int t_e, vector<std::string> path_till_here,
        std::set<int> candidates, vector<int> *cycleLengthArray, monitor *m) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    set<nodeid> V;
    set<pedge> remove_E;
    E = getFilteredData(w, t_s + 1, t_e, &candidates);

    for (auto x: E) {
        V.insert(x.toVertex);
        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] < x.time) {
                remove_E.insert(x);
            }
        }
    }
    for (auto x:remove_E) {
        E.erase(x);
    }

    m->edge_count += E.size();
    int cyclelenght;
    for (auto x: E) {
        if (candidates.count(x.toVertex) > 0) {
            V.insert(x.toVertex);
        }
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    m->cycles[cyclelenght]++;
                    std::string resultline = "Path," + to_string(cyclelenght) + ",";
                    (*cycleLengthArray)[cyclelenght] = (*cycleLengthArray)[cyclelenght] + 1;
                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here)[i];

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {
        if (ct.count(x) == 0) {
            ct[x] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x] = temp;
        }
        time_x = getAllTime(E, x);
        while (!time_x.empty()) {
            t_min = *time_x.begin();
            set<int> newcand = candidates;
            newcand.erase(x);
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));
            bool pathFound = allPath(x, rootnode, t_min + 1, t_e, newpath, newcand, cycleLengthArray, m);
            if (ct[x] <= t_min || !pathFound) {
                time_x.clear();
                U[x].insert(make_pair(w, t_min));
            } else {
                time_x.erase(t_min);
                if (t_min > lastp) {
                    lastp = t_min;
                }
            }
        }

    }
    if (lastp > 0) {
        unblock(w, lastp, t_e);
    }
    return (lastp > 0);
}

pathBundle expandPathBundle(pathBundle current_path, edgeBundle new_edge) {
    timeBundle T_last;
    pathBundle new_extended_path;
    for (int t: new_edge.time.times) {
        if (t > current_path.getLastEdge().time.getMinTime()) {
            T_last.times.insert(t);
        }
    }

    if (T_last.size() > 0) {
        //add the last edge with the new time bundle T_last in the current path to extend it.
        new_edge.time = T_last;
        new_extended_path.path = current_path.path;
        new_extended_path.path.push_back(new_edge);

        //update the time bundle on all the other edge accordingly
        timeBundle T_i;
        int k = current_path.path.size();
        for (int i = k - 1; i >= 0; i--) {
            for (int t: new_extended_path.path[i].time.times) {
                int max_t = new_extended_path.path[i + 1].time.getMaxTime();
                if (t < max_t) {
                    T_i.times.insert(t);
                }
            }
            new_extended_path.path[i].time = T_i;
            T_i.times.clear();
        }
    }
    return new_extended_path;
}

int pathCount(pathBundle pb) {
    std::priority_queue<pair<int, int>, vector<pair<int, int>>, myComparator> H_prev;
    H_prev.push(make_pair(0, 1));
    int cycle_length = pb.path.size();
    int n, prev, t, t_, n_;
    for (int i = 0; i < cycle_length; i++) {
        std::priority_queue<pair<int, int>, vector<pair<int, int>>, myComparator> H_i;
        n = 0;
        prev = 0;
        for (set<int>::iterator it = pb.path[i].time.times.begin(); it != pb.path[i].time.times.end(); it++) {
            t = *it;
            if (!H_prev.empty()) {
                pair<int, int> temp = H_prev.top();
                t_ = temp.first;
                n_ = temp.second;
                while (t_ < t) {
                    H_prev.pop();
                    n = n_;
                    if (H_prev.empty()) {
                        break;
                    } else {
                        temp = H_prev.top();
                        t_ = temp.first;
                        n_ = temp.second;
                    }
                }

            }
            H_i.push(make_pair(t, prev + n));
            prev = prev + n;
        }
        H_prev = H_i;
    }
    while (H_prev.size() > 1) {
        H_prev.pop();
    }
    return H_prev.top().second;

}

void testCountPath() {
    timeBundle tb;
    tb.times.insert(2530);
    // tb.times.insert(1196833908);

    edgeBundle eb;
    eb.from_node = 3170;
    eb.to_node = 11161;
    eb.time = tb;
    pathBundle pb;
    pb.path.push_back(eb);

    tb.times.clear();
    tb.times.insert(2531);
    //   tb.times.insert(1196833468);

    eb.from_node = 11161;
    eb.to_node = 13140;
    eb.time = tb;
    pb.path.push_back(eb);

    tb.times.clear();
    tb.times.insert(2532);
    //   tb.times.insert(1196833735);

    eb.from_node = 13140;
    eb.to_node = 2734;
    eb.time = tb;
    pb.path.push_back(eb);


    tb.times.clear();
    tb.times.insert(2533);
//    tb.times.insert(1196833986);
    eb.from_node = 2734;
    eb.to_node = 3170;
    eb.time = tb;
    pb.path.push_back(eb);


    tb.times.clear();
    tb.times.insert(2535);
    //   tb.times.insert(3252);
    eb.from_node = 2734;
    eb.to_node = 3170;
    eb.time = tb;
    pb.path.push_back(eb);

    int num_path = pathCount(pb);
    cout << num_path << endl;
}

int
allPathBundle(pathBundle path_bundle_till_here, int t_e, std::set<int> candidates, vector<int> *cycleLengthArray,
              monitor *m) {
    edgeBundle lastEdge = path_bundle_till_here.getLastEdge();
    int t_current = lastEdge.time.getMinTime();
    int v_current = lastEdge.to_node;
    int rootnode = path_bundle_till_here.getRootNode();
    ct[v_current] = t_current;
    int lastp = 0;

    set<nodeid> V;
    set<pedge> remove_E;
    set<pedge> E = getFilteredData(v_current, t_current + 1, t_e, &candidates);

    for (auto x: E) {

        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] <= x.time) {
                remove_E.insert(x);
            }
        }
    }
    for (auto x:remove_E) {
        E.erase(x);
    }
    if (E.size() == 0) {
        return lastp;
    } else {
        if (E.size() > max_E_count) {
            max_E_count = E.size();
        }
        m->edge_count += E.size();
    }

    int cyclelenght;
    for (auto x: E) {
        if (candidates.count(x.toVertex) > 0) {
            V.insert(x.toVertex);
        }
    }
    if (V.count(rootnode) > 0) {
        timeBundle T;
        T.times = getAllTime(E, rootnode);

        edgeBundle closing_edge;
        closing_edge.from_node = v_current;
        closing_edge.to_node = rootnode;
        closing_edge.time = T;
        pathBundle cycle = expandPathBundle(path_bundle_till_here, closing_edge);
        int cycle_length = cycle.path.size();
        if (cycle_length > 2) {
            // resultAllPath.insert(cycle.printPath());
            resultAllPathBundle[rootnode][cycle_length].push_back(cycle);
            m->cycles[cycle_length]++;
            int max_t = T.getMaxTime();
            if (max_t > lastp) {
                lastp = max_t;
            }

        }


    }
    V.erase(rootnode);
    timeBundle time_x;
    timeBundle new_time_x;
    int t_min;
    bool not_present;
    for (auto x: V) {
        if (ct.count(x) == 0) {
            ct[x] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x] = temp;
        }
        time_x.times = getAllTime(E, x);
        for (int t:time_x.times) {
            if (t < ct[x]) {
                new_time_x.times.insert(t);
            }
        }
        if (time_x.size() != 0) {

            edgeBundle eb;
            eb.time = time_x;
            eb.from_node = v_current;
            eb.to_node = x;
            pathBundle newPathBundle = expandPathBundle(path_bundle_till_here, eb);
            if (newPathBundle.path.size() != 0) {
                candidates.insert(x);
                int last_x = allPathBundle(newPathBundle, t_e, candidates, cycleLengthArray, m);
                if (last_x > lastp) {
                    lastp = last_x;
                }
                timeBundle F_x;
                for (int tempt:time_x.times) {
                    if (tempt > last_x) {
                        F_x.times.insert(tempt);
                    }
                }

                if (F_x.size() > 0) {
                    t_min = *F_x.times.begin();
                    not_present = true;
                    for (auto temp_pair:U[x]) {
                        if (temp_pair.first == v_current) {
                            if (temp_pair.second > t_min) {
                                U[x].erase(temp_pair);
                                U[x].insert(make_pair(v_current, t_min));

                            }
                            not_present = false;
                        }
                    }
                    if (not_present) {
                        U[x].insert(make_pair(v_current, t_min));
                    }


                }
            }
        }
    }
    if (lastp > 0) {
        unblock(v_current, lastp, t_e);
    }
    return lastp;
}

int
allPathBundleApprox(pathBundle path_bundle_till_here, int t_e, bloom_filter candidates, vector<int> *cycleLengthArray) {
    edgeBundle lastEdge = path_bundle_till_here.getLastEdge();
    int t_current = lastEdge.time.getMinTime();
    int v_current = lastEdge.to_node;
    int rootnode = path_bundle_till_here.getRootNode();
    set<int> seen = path_bundle_till_here.getAllNodes();
    ct[v_current] = t_current;
    int lastp = 0;

    set<nodeid> V;
    set<pedge> remove_E;
    set<pedge> E = getFilteredData(v_current, t_current + 1, t_e, &candidates);

    for (auto x: E) {

        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] <= x.time) {
                remove_E.insert(x);
            }
        }
    }

    for (auto x:remove_E) {
        E.erase(x);
    }
    if (E.size() == 0) {
        return lastp;
    } else {

        int temp = E.size();
        if (temp > max_E_count) {

            max_E_count = E.size();
        }
    }

    int cyclelenght;
    for (auto x: E) {
        if (candidates.contains(x.toVertex)) {
            if (seen.count(x.toVertex) == 0) {
                V.insert(x.toVertex);
            }

        }
    }
    if (V.count(rootnode) > 0) {
         string subgraph="twitter_mini_cycle_"+to_string(rootnode)+".txt";
          ofstream cycleFile;
           cycleFile.open(subgraph.c_str(),std::ofstream::app);
        timeBundle T;
        T.times = getAllTime(E, rootnode);
        int max_t = T.getMaxTime();
        if (max_t > lastp) {
            lastp = max_t;
        }
        if (path_bundle_till_here.path.size() > 1) {

              edgeBundle closing_edge;
               closing_edge.from_node = v_current;
               closing_edge.to_node = rootnode;
               closing_edge.time = T;
               pathBundle cycle = expandPathBundle(path_bundle_till_here, closing_edge);
             int cycle_length = cycle.path.size();
             if (cycle_length > 2) {
                 cycle_count++;
                 if (cycle_count % 100000 == 0) {
                     cout << "cycles found: " << cycle_count << endl;
                 }
            // resultAllPath.insert(cycle.printPath());
                   cycleFile<<cycle.printPath()<<"\n";
            //  resultAllPathBundle[rootnode][path_bundle_till_here.path.size()+1].push_back(null);


              }
        }
           cycleFile.flush();
           cycleFile.close();
    }

    V.erase(rootnode);
    timeBundle time_x;
    timeBundle new_time_x;
    int t_min;
    bool not_present;

    for (auto x: V) {

        if (ct.count(x) == 0) {
            ct[x] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x] = temp;
        }
        time_x.times = getAllTime(E, x);
        for (int t:time_x.times) {
            if (t < ct[x]) {
                new_time_x.times.insert(t);
            }
        }
        if (time_x.size() != 0) {

            edgeBundle eb;
            eb.time = time_x;
            eb.from_node = v_current;
            eb.to_node = x;
            pathBundle newPathBundle = expandPathBundle(path_bundle_till_here, eb);
            if (newPathBundle.path.size() != 0 & newPathBundle.getAllNodes().size() == newPathBundle.path.size()) {
                // cout << x << " ";
                int last_x = allPathBundleApprox(newPathBundle, t_e, candidates, cycleLengthArray);
                if (last_x > lastp) {
                    lastp = last_x;
                }
                timeBundle F_x;
                for (int tempt:time_x.times) {
                    if (tempt > last_x) {
                        F_x.times.insert(tempt);
                    }
                }

                if (F_x.size() > 0) {
                    t_min = *F_x.times.begin();
                    not_present = true;
                    for (auto temp_pair:U[x]) {
                        if (temp_pair.first == v_current) {
                            if (temp_pair.second > t_min) {
                                U[x].erase(temp_pair);
                                U[x].insert(make_pair(v_current, t_min));

                            }
                            not_present = false;
                        }
                    }
                    if (not_present) {
                        U[x].insert(make_pair(v_current, t_min));
                    }


                }
            }
        }
    }
    if (lastp > 0) {
        unblock(v_current, lastp, t_e);
    }
    return lastp;
}


bool
allPathWithoutCandidate(nodeid w, nodeid rootnode, int t_s, int t_e, vector<std::string> path_till_here,
                        std::set<int> seen_node, vector<int> *cycleLengthArray) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    E = getFilteredData(w, t_s + 1, t_e);


    set<nodeid> V;
    int cyclelenght;
    for (auto x: E) {
        if (seen_node.count(x.toVertex) == 0) {
            V.insert(x.toVertex);
        }
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    std::string resultline = "Path," + to_string(cyclelenght) + ",";
                    (*cycleLengthArray)[cyclelenght] = (*cycleLengthArray)[cyclelenght] + 1;
                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here)[i];

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {
        if (ct.count(x) == 0) {
            ct[x] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x] = temp;
        }
        time_x = getAllTime(E, x);
        while (!time_x.empty()) {
            t_min = *time_x.begin();
            set<int> new_seen_node = seen_node;
            new_seen_node.insert(x);
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));
            bool pathFound = allPathWithoutCandidate(x, rootnode, t_min + 1, t_e, newpath, new_seen_node,
                                                     cycleLengthArray);
            if (ct[x] <= t_min || !pathFound) {
                time_x.clear();
                U[x].insert(make_pair(w, t_min));
            } else {
                time_x.erase(t_min);
                if (t_min > lastp) {
                    lastp = t_min;
                }
            }
        }

    }
    if (lastp > 0) {
        unblock(w, lastp, t_e);
    }
    return (lastp > 0);
}

/**
 * Get all time t for node x such that *,x,t is in E
 * @param E
 * @param dst
 * @return set<time>
 */
set<int> getAllTime(set<pedge> E, nodeid dst) {
    set<int> times;
    for (auto x: E) {
        if (x.toVertex == dst) {
            times.insert(x.time);
        }
    }
    return times;
}


string DynamicDFS(nodeid rootnode, int t_s, int t_end, std::set<int> candidates, int window_bracket,
                  bool isCompressed, bool candidates_provided, bool use_bundle, vector<int> *cycleLengthArray) {
    candidates.insert(rootnode);
    string monitor_result = "";
    monitor m;
    Platform::Timer monitor_timer;
    set<pedge> neighbours;
    ct.clear();
    U.clear();
    if (isCompressed) {
        neighbours = getFilteredData(rootnode, t_s,
                                     t_end);// all the edges of type rootnode,*,t where t is between t_s and t_end
    } else {
        neighbours = getFilteredData(rootnode, t_s);// all the edges of type rootnode,*,t_s
    }
    // cout<<"finding cycle for: "<<rootnode<<" neighbours "<<neighbours.size()<<endl;
    if (use_bundle) {
        timeBundle tb;
        set<int> V;

        for (auto x:neighbours) {
            if (candidates.count(x.toVertex) > 0) {
                V.insert(x.toVertex);
            }
        }
        for (auto x:V) {

            tb.times = getAllTime(neighbours, x);
            if (tb.times.size() > 0) {
                m.clear();
                set<int> tempcanidates = candidates;
                tempcanidates.erase(x);
                pathBundle path_bundle_till_here;
                edgeBundle eb;
                eb.from_node = rootnode;
                eb.to_node = x;
                eb.time = tb;

                path_bundle_till_here.path.push_back(eb);
                monitor_timer.Start();
                m.candidate_count = tempcanidates.size() - 1;
                allPathBundle(path_bundle_till_here, t_end, tempcanidates, cycleLengthArray, &m);
                monitor_result =
                        monitor_result + to_string(rootnode) + "," + to_string(x) + "," + to_string(m.candidate_count) +
                        "," + to_string(m.edge_count) + "," + to_string(monitor_timer.LiveElapsedMilliseconds());
                monitor_timer.Stop();
                int max_cycle = 0;
                for (int i = 0; i < 50; i++) {
                    if (m.cycles[i] > 0) {
                        if (i > max_cycle) {
                            max_cycle = i;
                        }
                    }
                }
                for (int j = 2; j <= max_cycle; ++j) {
                    monitor_result = monitor_result + "," + to_string(j) + ":" + to_string(m.cycles[j]);
                }
                monitor_result = monitor_result + "\n";

            }

        }
    } else {
        for (auto x:neighbours) {
            if (candidates.count(x.toVertex) > 0) {

                if (candidates_provided) {
                    std::set<int> tempcandidate = candidates;

                    tempcandidate.erase(x.toVertex);
                    vector<std::string> path_till_here;
                    path_till_here.push_back(
                            to_string(rootnode) + "," + to_string(x.toVertex) + "," + to_string(x.time));
                    monitor_timer.Start();
                    allPath(x.toVertex, rootnode, x.time + 1, t_end, path_till_here, tempcandidate, cycleLengthArray,
                            &m);
                    monitor_result =
                            monitor_result + to_string(rootnode) + "," + to_string(x.toVertex) + "," +
                            to_string(m.candidate_count) +
                            "," + to_string(m.edge_count) + "," + to_string(monitor_timer.LiveElapsedMilliseconds());
                    monitor_timer.Stop();
                    int max_cycle = 0;
                    for (int i = 0; i < 50; i++) {
                        if (m.cycles[i] > 0) {
                            if (i > max_cycle) {
                                max_cycle = i;
                            }
                        }
                    }
                    for (int j = 3; j <= max_cycle; ++j) {
                        monitor_result = monitor_result + "," + to_string(j) + ":" + to_string(m.cycles[j]);
                    }
                    monitor_result = monitor_result + "\n";
                } else {
                    std::set<int> seen_node;
                    seen_node.insert(x.toVertex);

                    vector<std::string> path_till_here;
                    path_till_here.push_back(
                            to_string(rootnode) + "," + to_string(x.toVertex) + "," + to_string(x.time));
                    allPathWithoutCandidate(x.toVertex, rootnode, x.time + 1, t_end, path_till_here, seen_node,
                                            cycleLengthArray);
                }

            }
        }
    }
    return monitor_result;
}


void findAllCycleNaive(std::string inputGraph, std::string resultFile, int window, bool reverseEdge) {
    int window_bracket = window * 60 * 60;
    double ptime = 0.0;
    int count = 0;

    string line;
    Platform::Timer timer;
    timer.Start();

    int cyclecount = 0;
    std::vector<std::string> templine;
    ifstream infile(inputGraph.c_str());
    int src, dst;
    int t_s;
    int i = 0;
    map<nodeid, vector<tpath>> allpaths;
    map<nodeid, vector<tpath>>::iterator pathiterator;
    set<string> resultPath;
    ofstream resultOut;
    resultOut.open(resultFile.c_str());
    int selfloop = 0;
    //  map<nodeid,vector<pair<vector<pedge>,set<nodeid>>>*> pathendpointers;
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        set<string> candidateset;
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        t_s = stol(templine[2].c_str());
        if (src != dst) {
            pedge newedge;
            newedge.fromVertex = src;
            newedge.toVertex = dst;
            newedge.time = t_s;
            tpath newpath;
            newpath.path.push_back(newedge);
            newpath.t_start = t_s;
            newpath.rootnode = src;
            newpath.seen.insert(dst);
            newpath.seen.insert(src);
            allpaths[dst].push_back(newpath);

            //get all the paths ending with dst
            if (allpaths.count(src) > 0) {
                for (int index = 0; index < allpaths[src].size(); index++) {
                    if (t_s - allpaths[src][index].t_start > window_bracket) {
                        allpaths[src].erase(allpaths[src].begin() + index);
                        index--;
                    } else {
                        if (allpaths[src][index].rootnode == dst) {//root of the path is destination
//cycle found
                            if (allpaths[src][index].path.size() > 1) {
                                string path = "Path, " + to_string(allpaths[src][index].path.size() + 1) + " , ";
                                cyclecount++;
                                for (int j = 0; j < allpaths[src][index].path.size(); j++) {

                                    path = path + to_string(allpaths[src][index].path[j].fromVertex) + ","
                                           + to_string(allpaths[src][index].path[j].toVertex) + ","
                                           + to_string(allpaths[src][index].path[j].time) + ",";

                                }
                                path = path + to_string(src) + "," + to_string(dst) + "," + to_string(t_s);
                                //    std::cout << "->" << line << endl;
                                resultPath.insert(path);
                            }
                        } else if (allpaths[src][index].seen.count(dst) == 0) {// dst is not yet seen
                            tpath extendedpath;
                            extendedpath.rootnode = allpaths[src][index].rootnode;
                            extendedpath.seen = allpaths[src][index].seen;
                            extendedpath.seen.insert(dst);
                            extendedpath.t_start = allpaths[src][index].t_start;
                            extendedpath.path = allpaths[src][index].path;
                            extendedpath.path.push_back(newedge);
                            allpaths[dst].push_back(extendedpath);
                        }
                    }
                }


            }

        } else {
            //self loop
            selfloop++;
        }
        count++;
        if (count % 10000 == 0) {


            std::cout << "finished parsing, count," << count << " , " << timer.LiveElapsedSeconds() - ptime
                      << ", ";

            std::cout << allpaths.size() << " Memory, " << getMem() << " Cycle, " << cyclecount << endl;
            ptime = timer.LiveElapsedSeconds();
        }
    }

    for (string temp:resultPath) {
        resultOut << temp << endl;
    }
    resultOut.close();
    std::cout << "self loop, " << selfloop << endl;
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds() << " Cycle, " << resultPath.size()
              << std::endl;

    timer.Stop();


}


void
DynamicDFSApprox(approxCandidatesNew candidate, int window_bracket, bool use_bundle, vector<int> *cycleLengthArray) {
    ct.clear();
    U.clear();

    int root_node = candidate.root_node;


    if (use_bundle) {
        set<pedge> neighbours;

        neighbours = getFilteredData(root_node, candidate.start_time,
                                     candidate.end_time);// all the edges of type rootnode,*,t where t is between t_s and t_end

        timeBundle tb;
        set<int> V;


        for (auto x:neighbours) {
            if (candidate.neighbours_candidates.count(x.toVertex) > 0) {
                V.insert(x.toVertex);

                if (x.toVertex != root_node) {
                    ct[x.toVertex] = std::numeric_limits<int>::max();
                }
            }
        }

        V.erase(root_node);
        //    cout<<"neighbours: "<<V.size()<<endl;
        for (auto x:V) {

            //  ct.clear();
            //    U.clear();
            tb.times = getAllTime(neighbours, x);
            if (tb.times.size() > 0) {
//cout<<"neig "<<x<<" : ";

                pathBundle path_bundle_till_here;
                edgeBundle eb;
                eb.from_node = root_node;
                eb.to_node = x;
                eb.time = tb;
                path_bundle_till_here.path.push_back(eb);

                allPathBundleApprox(path_bundle_till_here, candidate.end_time, candidate.neighbours_candidates[x],
                                    cycleLengthArray);
                //cout<<endl;
            }

        }
    }

}

void DynamicDFSExact(exactCandidates candidate, int window_bracket, bool use_bundle, vector<int> *cycleLengthArray) {
    ct.clear();
    U.clear();

    monitor m;
    int root_node = candidate.root_node;


    if (use_bundle) {
        std::set<int> tempcandidate;
        map<int, edgeBundle> V;
        for (auto z:candidate.neighbours_time) {
            if (V.count(z.first) > 0) {
                V[z.first].time.times.insert(z.second);
            } else {

                edgeBundle eb;
                eb.from_node = root_node;
                eb.to_node = z.first;
                eb.time.times.insert(z.second);
                V[z.first] = eb;

            }

        }
        if (V.size() > 0) {

            for (map<int, edgeBundle>::iterator iterator1 = V.begin(); iterator1 != V.end(); iterator1++) {
                ct.clear();
                U.clear();
                tempcandidate = candidate.candidates_nodes;
                tempcandidate.erase(iterator1->first);
                pathBundle path_bundle_till_here;
                path_bundle_till_here.path.push_back(iterator1->second);
                allPathBundle(path_bundle_till_here, candidate.end_time, tempcandidate, cycleLengthArray, &m);
            }

        }


    } else {
        for (auto x:candidate.neighbours_time) {
            ct.clear();
            U.clear();
            vector<std::string> path_till_here;
            path_till_here.push_back(to_string(root_node) + "," + to_string(x.first) + "," + to_string(x.second));
            allPathExact(x.first, root_node, x.second, candidate.end_time, path_till_here, candidate.candidates_nodes);
        }
    }


}

bool
allPathApprox(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
              bloom_filter candidates) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    set<pedge> remove_E;

    E = getFilteredData(w, t_s + 1, t_e, &candidates);
    set<nodeid> V;
    int cyclelenght;
    for (auto x: E) {
        V.insert(x.toVertex);
        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] <= x.time) {
                remove_E.insert(x);
            }
        }
    }
    for (auto x:remove_E) {
        E.erase(x);
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    std::string resultline = "Path," + to_string(cyclelenght) + ",";

                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here[i]);

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {

        time_x = getAllTime(E, x);
        while (!time_x.empty()) {
            t_min = *time_x.begin();
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));
            bool pathFound = allPathApprox(x, rootnode, t_min + 1, t_e, newpath, candidates);
            if (ct[x] <= t_min || !pathFound) {
                time_x.clear();
                U[x].insert(make_pair(w, t_min));
            } else {
                time_x.erase(t_min);
                if (t_min > lastp) {
                    lastp = t_min;
                }
            }
        }

    }
    if (lastp > 0) {
        unblock(w, lastp, t_e);
    }
    return (lastp > 0);
}

bool
allPathExact(int w, int rootnode, int t_s, int t_e, vector<std::string> path_till_here,
             set<int> candidates) {
    ct[w] = t_s;
    int lastp = 0;
    set<pedge> E;
    set<pedge> remove_E;

    E = getFilteredData(w, t_s + 1, t_e, &candidates);
    set<nodeid> V;
    int cyclelenght;
    for (auto x: E) {

        V.insert(x.toVertex);
        if (ct.count(x.toVertex) == 0) {// if seen for the first time initialize close time and dependent list
            ct[x.toVertex] = std::numeric_limits<int>::max();
            set<pair<nodeid, int>> temp;
            U[x.toVertex] = temp;
        } else {
            if (ct[x.toVertex] <= x.time) {
                int temp = ct[x.toVertex];
                remove_E.insert(x);
            }
        }

    }
    for (auto x:remove_E) {
        E.erase(x);
    }
    if (V.count(rootnode) > 0) {
        for (auto x: E) {
            if (x.toVertex == rootnode) {
                if (x.time > lastp) {
                    lastp = x.time;
                }
                if (path_till_here.size() + 1 > 2) {
                    //   std::cout << "Found cycle, " << path_till_here.size() + 1 << " , ";
                    cyclelenght = path_till_here.size() + 1;
                    std::string resultline = "Path," + to_string(cyclelenght) + ",";

                    for (int i = 0; i < path_till_here.size(); i++) {
                        // std::cout << "->" << (path_till_here)[i];
                        resultline = resultline + "," + (path_till_here[i]);

                    }
                    //  std::cout << "->" << w << "," << rootnode << "," << x.time << endl;
                    resultline = resultline + "," + to_string(w) + "," + to_string(rootnode) + "," + to_string(x.time);
                    resultAllPath.insert(resultline);
                }
            }
        }
    }
    V.erase(rootnode);
    set<int> time_x;
    int t_min;
    for (auto x: V) {

        time_x = getAllTime(E, x);

        while (!time_x.empty()) {
            t_min = *time_x.begin();
            vector<std::string> newpath = path_till_here;
            newpath.push_back(to_string(w) + "," + to_string(x) + "," + to_string(t_min));

            bool pathFound = allPathExact(x, rootnode, t_min + 1, t_e, newpath, candidates);
            if (ct[x] <= t_min || !pathFound) {
                time_x.clear();
                U[x].insert(make_pair(w, t_min));
            } else {
                time_x.erase(t_min);
                if (t_min > lastp) {
                    lastp = t_min;
                }
            }
        }

    }
    if (lastp > 0) {
        unblock(w, lastp, t_e);
    }
    return (lastp > 0);
}
