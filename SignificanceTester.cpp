//
// Created by Rohit on 21-Dec-17.
//

#include "SignificanceTester.h"
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include "Split.h"
#include <set>
#include <iostream>
#include<bits/stdc++.h>

using namespace std;
std::map<std::string, set<int>> data;
std::map<std::string, int> all_signatures;

void prepareData(std::string dataFile, std::string pathFile, bool reverseEdge) {
    std::vector<std::string> templine;
    ifstream infile(dataFile.c_str());
    int src, dst;
    int timestamp;
    string line;
    string key;
    while (infile >> line) {

        templine = Tools::Split(line, ',');
        src = stoi(templine[0]);
        dst = stoi(templine[1]);
        if (reverseEdge) {
            src = stoi(templine[1]);
            dst = stoi(templine[0]);
        }
        timestamp = stol(templine[2].substr(0, 10).c_str());
        key = to_string(src) + "-" + to_string(dst);

        data[key].insert(timestamp);

    }
    cout << "parsed all data" << endl;
    ifstream pfile(pathFile.c_str());
    int length;
    int idx;
    string signature = "";
    while (pfile >> line) {
        signature = "";

        templine = Tools::Split(line, ',');
        length = stoi(templine[1]);
        idx = 2;
        while (idx < length * 3) {
            signature = signature + templine[idx + 1] + "-" + templine[idx + 2] + ",";
            idx = idx + 3;
        }
        if (all_signatures.count(signature) == 0) {
            all_signatures[signature] = 0;

        }
        all_signatures[signature]++;

    }
    cout << "# of unique signatures:: " << all_signatures.size() << endl;
}

void getSignificantCycle(int window, string output) {
    int window_bracket = window * 60 * 60;
    std::vector<std::string> tempsig;
    int length;
    int significant_count=0;
    int not_significant_count=0;
    for (map<string, int>::iterator it = all_signatures.begin(); it != all_signatures.end(); ++it) {
        tempsig = Tools::Split(it->first, ',');
        length = tempsig.size();
        vector<double> freq_table;
        vector<set<int>> time_table;
        set<int> T;
        double P = 1.0;
        double T_mod = 0.0;
        double Ti_mod = 0.0;
        P = 1.0;
        for (int i = 0; i < length; ++i) {
            time_table.push_back(data[tempsig[i]]);
            T.insert(data[tempsig[i]].begin(), data[tempsig[i]].end());
        }
        T_mod = T.size();
        for (int i = 0; i < time_table.size(); i++) {
            Ti_mod = time_table[i].size();
            P = P * (Ti_mod / T_mod);
        }



        bool is_first = true;
        set<int> W;
        int temp_t;
        int t_next;
        int t_1 = *T.begin();
        int cnt = 0;
        for (set<int>::iterator it = T.begin(); it != T.end(); ++it) {
            temp_t = *it;
            if (is_first) {
                if (temp_t - t_1 <= window_bracket) {
                    W.insert(temp_t);
                } else {
                    cnt = binomialCoeff(W.size(), length);
                    t_next = temp_t;
                    W.clear();
                    W.insert(t_next);
                    is_first = false;
                }
            } else {
                if (temp_t - t_next <= window_bracket) {
                    W.insert(temp_t);
                } else {
                    cnt =cnt+ binomialCoeff(W.size()-1, length-1);
                    t_next = temp_t;
                    W.clear();
                    W.insert(t_next);

                }
            }

        }
        if(is_first){
            cnt = binomialCoeff(W.size(), length);
        }else{
            if(W.size()>0){
                cnt =cnt+ binomialCoeff(W.size()-1, length-1);
                W.clear();
            }
        }

        double expected_cnt=P*cnt;


        if(it->second>ceil(expected_cnt)){
            cout << "P, " << P ;
            cout<<" count, "<<cnt;
            cout<<" cnt time p ,"<<expected_cnt;
            cout<<" actual count, "<<it->second;

            cout<<" length, "<<length<<endl;


            significant_count++;
        }else{
           // cout<<"non significant cycle of length"<<length<<" : "<<it->second<<endl;
            not_significant_count++;
        }


    }
    cout<<"significant count : "<<significant_count<<endl;
    cout<<"not significant count : "<<not_significant_count<<endl;


}

int binomialCoeff(int n, int k) {
    int C[k + 1];
    memset(C, 0, sizeof(C));

    C[0] = 1;  // nC0 is 1

    for (int i = 1; i <= n; i++) {
        // Compute next row of pascal triangle using
        // the previous row
        for (int j = min(i, k); j > 0; j--)
            C[j] = C[j] + C[j - 1];
    }
    return C[k];
}