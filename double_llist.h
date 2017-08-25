//
// Created by Rohit on 24-Aug-17.
//
#include<iostream>
#include <map>
#include <set>
#include <vector>
#include "DetectCycle.h"

#ifndef TEST_DOUBLE_LLIST_H
#define TEST_DOUBLE_LLIST_H
/*
 * Node Declaration
 */
using namespace std;
struct node
{
    long updatetime;
    string nodeid;
    struct node *next;
    struct node *prev;
};


/*
 Class Declaration
 */
class double_llist
{
public:
    void create_list(long update_time,string nodeid);
    node * add_begin(long update_time,string nodeid);
    void display_dlist();
    void count();
    void reverse();
    void delete_element(node *this_node);
    vector<string> get_expired_nodes(long current_time);
    int delete_expired(long current_time);

};


#endif //TEST_DOUBLE_LLIST_H
