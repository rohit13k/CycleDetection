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
    int updatetime;
    int nodeid;
    struct node *next;
    struct node *prev;
};


/*
 Class Declaration
 */
class double_llist
{
public:
    void create_list(int update_time,int nodeid);
    node * add_begin(int update_time,int nodeid);
    void display_dlist();
    void count();
    void reverse();
    void delete_element(node *this_node);
    vector<int> get_expired_nodes(int current_time);
    int delete_expired(int current_time);

};


#endif //TEST_DOUBLE_LLIST_H
