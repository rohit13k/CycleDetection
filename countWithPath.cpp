//
// Created by Rohit on 16-Jun-17.
//

#include "countWithPath.h"
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
typedef string nodeid;
int countWithPath(std::string input, std::string output, int window,bool timeInMsec,int cleanUpLimit,int cyclelenght) {
    std::vector<std::string> templine;
    typedef long time;

    typedef string path;
    typedef map<nodeid,path> node_set;
    typedef map<time, node_set> time_group;
    typedef time_group distance_summary[100];
    map<nodeid, distance_summary> completeSummary;
    std:: map<nodeid, distance_summary> ::iterator it;
    std::map<time, node_set>::iterator itTimedSet;
    set<string> ::iterator it_impnode;
    ifstream infile(input.c_str());
    string src, dst;
    string line;
    Platform::Timer timer;
    timer.Start();
    long timestamp;
    int count = 0;
    double ptime = 0.0;
    double etime=0.0;
    long cycleCounter[100]={};
    long negativeTimestamp;
    long window_bracket=window  * 60 *60;
    string tempnode="";
    string temppath="";
    if(timeInMsec){
        window_bracket=window_bracket*1000;
    }
    set<string> impnode[100]={};
    while (infile >> line) {
        templine = Tools::Split(line, ',');
        src = templine[0];
        dst = templine[1];

        timestamp = stol(templine[2].c_str());
        negativeTimestamp=-1*timestamp;
        int erasedCount=0;
        if (src.compare(dst)!=0) {
            temppath="("+line+")";
            //if exist update summary else create initial instance
            if (completeSummary.count(dst) > 0) {
                //add the src in dst at distance 0
                for ( itTimedSet = completeSummary[dst][0].begin();
                      itTimedSet != completeSummary[dst][0].end(); ++itTimedSet) {
                    if(itTimedSet->second.count(src)>0){
                        completeSummary[dst][0][itTimedSet->first].erase(src);
                    }
                }
                completeSummary.find(dst)->second[0][negativeTimestamp][src]=temppath;
            } else {

                    completeSummary.find(dst)->second[0][negativeTimestamp][src]="";



            }
            //if src summary exist put i_th summary of src in i+1_th summary of dst
            //if dst exist at i_th summary of src increase count of cycle with lenght i+1
            if (completeSummary.count(src) > 0) {
                bool print=false;
                for (int i = 0; i < 99; i++) {
                    if(i+1>10){
                        print=true;
                    }
                    for ( itTimedSet = completeSummary.find(src)->second[i].begin();
                          itTimedSet != completeSummary.find(src)->second[i].end(); ++itTimedSet) {
                        //if out of window remove everything from current till end
                        if(timestamp-(-1*itTimedSet->first)>window_bracket){
                            completeSummary.find(src)->second[i].erase(itTimedSet,completeSummary.find(src)->second[i].end());
                            erasedCount++;
                            break;
                        }
                        for (std::map<nodeid,path>::iterator itnode= itTimedSet->second.begin(); itnode!= itTimedSet->second.end(); ++itnode){
                            tempnode=itnode->first;
                            temppath=itnode->second+"("+line+")";
                            if(tempnode.compare(dst)==0){
                                //the dst is already present in the summary at distance i
                                cycleCounter[i+1]++;
                                if(i+1>cyclelenght-1){
                                    impnode[i+1].insert(dst);
                                    cout<<temppath<<endl;
                                }

                            }else{
                                completeSummary.find(dst)->second[i+1][itTimedSet->first][tempnode]=temppath;


                            }
                        }
                    }
                }

            }
        }else{
            cycleCounter[0]++;
        }
        count++;

        if (count%cleanUpLimit==0) {
            double parseTime=timer.LiveElapsedSeconds()-ptime;
            ptime=timer.LiveElapsedSeconds();
            for (it=completeSummary.begin(); it!=completeSummary.end(); ++it){

                for (int i = 0; i < 99; i++) {

                    for (itTimedSet = completeSummary.find(it->first)->second[i].begin();
                         itTimedSet != completeSummary.find(it->first)->second[i].end(); ++itTimedSet) {
                        //  std::cout <<  timestamp - (-1 * itTimedSet->first)<< std::endl;
                        if (timestamp - (-1 * itTimedSet->first) > window_bracket) {
                            completeSummary.find(it->first)->second[i].erase(itTimedSet, completeSummary.find(
                                    it->first)->second[i].end());
                            erasedCount++;
                            break;
                        }
                    }
                }

            }

            double eraseTime=timer.LiveElapsedSeconds()-ptime;
            ptime=timer.LiveElapsedSeconds();
            std::cout << "finished parsing, count," << count<<"," << parseTime<<",erasecount,"<<erasedCount<<","<<eraseTime << std::endl;
            erasedCount=0;
        }



    }
    std::cout << "finished parsing all " << timer.LiveElapsedSeconds()
              << std::endl;
    timer.Stop();
    for(int k=cyclelenght-1;k<99;k++){
        if(impnode[k].size()>0){
            std::cout<<k;
            for(it_impnode=impnode[k].begin();it_impnode!=impnode[k].end();++it_impnode){
                std::cout<<" , "<<*it_impnode;
            }
            std::cout<<"" <<std::endl;
        }
    }
    ofstream result;
    result.open(output.c_str());
    for (int n :cycleCounter) {
        result << n << "\n";

    }

    result.close();

    timer.Stop();
    return 0;
}