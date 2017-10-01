//
// Created by Rohit on 24-Aug-17.
//

#include "double_llist.h"

node *start;
node *endNode;

/*
 * Create Double Link List
 */
void double_llist::create_list(int updatetime, int nodeid) {
    struct node *s, *temp;
    temp = new(struct node);
    temp->updatetime = updatetime;
    temp->nodeid = nodeid;
    temp->next = NULL;
    if (start == NULL) {
        temp->prev = NULL;
        start = temp;
        endNode = start;
    } else {
        s = start;
        while (s->next != NULL)
            s = s->next;
        s->next = temp;
        temp->prev = s;
        endNode = temp;
    }
}

/*
*  delete the given node from list
*/
void double_llist::delete_element(node *thisNode) {
    if(thisNode->next==NULL){
        //last element in the list is being deleted
        endNode=thisNode->prev;
    }
   thisNode->prev->next = thisNode->next;

    free(thisNode);
}

/*
 * Insertion at the beginning
 */
node * double_llist::add_begin(int updatetime, int nodeid) {
    if (start == NULL) {
        create_list(updatetime,  nodeid);
        return start;
    }
    struct node *temp;
    temp = new(struct node);
    temp->prev = NULL;
    temp->updatetime = updatetime;

    temp->nodeid = nodeid;
    temp->next = start;
    start->prev = temp;
    start = temp;
    return start;
}


/*
 * Display elements of Doubly Link List
 */
void double_llist::display_dlist() {
    struct node *q;
    if (start == NULL) {
        cout << "List empty,nothing to display" << endl;
        return;
    }
    q = start;
    cout << "The Doubly Link List is :" << endl;
    while (q != NULL) {
        cout << q->updatetime << "," << q->nodeid << " <-> ";
        q = q->next;
    }
    cout << "NULL" << endl;
}

/*
 * Number of elements in Doubly Link List
 */
void double_llist::count() {
    struct node *q = start;
    int cnt = 0;
    while (q != NULL) {
        q = q->next;
        cnt++;
    }
    cout << "Number of elements are: " << cnt << endl;
}

/*
 * Reverse Doubly Link List
 */
void double_llist::reverse() {
    struct node *p1, *p2;
    p1 = start;
    p2 = p1->next;
    p1->next = NULL;
    p1->prev = p2;
    while (p2 != NULL) {
        p2->prev = p2->next;
        p2->next = p1;
        p1 = p2;
        p2 = p2->prev;
    }
    start = p1;
    cout << "List Reversed" << endl;
}

/*
 * Delete all node from bottom till the time given
 */
int double_llist::delete_expired(int current_time) {
    struct node *q, *temp;
    int deleted_count=0;
    if (start == NULL) {
        cout << "List empty,nothing to delete" << endl;
        return deleted_count;
    }
    q = endNode;


    while (q != NULL) {
        temp = q;
        q = q->prev;
        if (temp->updatetime < current_time) {
            q->next = NULL;
            temp->prev = NULL;
            deleted_count++;

            free(temp);
        } else {
            return deleted_count;
        }

    }
    return deleted_count;
}


vector<int> double_llist::get_expired_nodes(int current_time) {
    struct node *q;
    vector<int>  expired_node;
    if (start == NULL) {
        cout << "List empty,nothing to delete" << endl;
        return expired_node;
    }
    q = endNode;


    while (q->prev != NULL) {


        if (q->updatetime < current_time) {
           expired_node.push_back(q->nodeid);
            q = q->prev;
        } else {
            return expired_node;
        }

    }
    return expired_node;
}