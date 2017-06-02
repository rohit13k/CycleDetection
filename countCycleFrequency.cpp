//
// Created by Rohit on 01-Jun-17.
//

#include "countCycleFrequency.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "Split.h"
#include "Timer.h"
#include <map>
#include <set>

using namespace std;

int count(std::string input, std::string output, int window) {
    std::vector<std::string> templine;
    typedef long time;
    typedef string nodeid;
    typedef set<nodeid> node_set;
    typedef map<time, node_set> time_group;
    typedef time_group distance_summary[100];
    map<nodeid, distance_summary> completeSummary;
    std:: map<nodeid, distance_summary> ::iterator it;
    std::map<time, set<nodeid>>::iterator itTimedSet;
    ifstream infile(input.c_str());
    string src, dst;
    string line;
    Platform::Timer timer;
    timer.Start();
    long timestamp;
    int count = 0;
    long ptime = 0l;
    long cycleCounter[100]={};
    long negativeTimestamp;
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = templine[0];
        dst = templine[1];
        timestamp = stol(templine[2].c_str());
        negativeTimestamp=-1*timestamp;
        int erasedCount=0;
        if (src != dst) {
            //if exist update summary else create initial instance
            if (completeSummary.count(dst) > 0) {
                //add the src in dst at distance 0
                completeSummary.find(dst)->second[0][negativeTimestamp].insert(src);
            } else {
                completeSummary[dst][0][negativeTimestamp].insert(src);

            }

                //if src summary exist put i_th summary of src in i+1_th summary of dst
                //if dst exist at i_th summary of src increase count of cycle with lenght i+1
                if (completeSummary.count(src) > 0) {

                    for (int i = 0; i < 98; i++) {

                        for ( itTimedSet = completeSummary.find(src)->second[i].begin();
                             itTimedSet != completeSummary.find(src)->second[i].end(); ++itTimedSet) {
                            if(timestamp-(-1*itTimedSet->first)>window*60*60*1000){
                                completeSummary.find(src)->second[i].erase(itTimedSet,completeSummary.find(src)->second[i].end());
                                erasedCount++;
                                break;
                            }


                            for (std::set<nodeid>::iterator itnode= itTimedSet->second.begin(); itnode!= itTimedSet->second.end(); ++itnode){
                                if(*itnode==dst){
                                    //the dst is already present in the summary at distance i
                                    cycleCounter[i+1]++;

                                }else{
                                    completeSummary.find(dst)->second[i+1][itTimedSet->first].insert(*itnode);

                                }
                            }

                        }


                    }
                }


        }else{
            cycleCounter[0]++;
        }

        count++;

        if (count%10000==0) {

            std::cout << "finished parsing, " << count<<"," << timer.LiveElapsedSeconds() - ptime << std::endl;
            for (it=completeSummary.begin(); it!=completeSummary.end(); ++it){
                for (int i = 0; i < 98; i++) {
                    for (itTimedSet = completeSummary.find(it->first)->second[i].begin();
                         itTimedSet != completeSummary.find(it->first)->second[i].end(); ++itTimedSet) {
                      //  std::cout <<  timestamp - (-1 * itTimedSet->first)<< std::endl;
                        if (timestamp - (-1 * itTimedSet->first) > window  * 60 *60) {
                            completeSummary.find(it->first)->second[i].erase(itTimedSet, completeSummary.find(
                                    it->first)->second[i].end());
                            erasedCount++;
                            break;
                        }
                    }
                }
            }
            std::cout << "erased count, " << erasedCount << std::endl;
            erasedCount=0;
            ptime = timer.LiveElapsedSeconds();
        }
    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    ofstream result;
    result.open(output.c_str());
    for (int n :cycleCounter) {
        result << n << "\n";

    }

    result.close();

    timer.Stop();
    return 0;
}