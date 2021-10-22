#include "api.h"
// api.h must be the first line

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.

int n_mails, n_queries;
mail *mails;
query *queries;
query *qlist, *tmp_qlist;

#define CHAR_SIZE 36

typedef struct node
{
    bool *index;
    struct node **child;
    int *txt;
    int tid; // token id
} Node;

typedef struct mnode
{
    bool index;
    struct mnode **child;
    int *txt;
    int tid; // token id
} mNode;

typedef struct lnode
{
   int data;
   struct lnode* next;
} Lnode;

typedef struct linklist {
    Lnode* back;
    Lnode* front;
} Linklist;

// define root & current node
Node *sroot; // single-tree root
Node *current;
Node *current2;

// define tables
int token_id;
int token_cnt; // for kot table building
Linklist **m2t; // mail_id to token
Linklist **t2m; // token code to mail_id

int *kot; // kinds of token
double *sim_val; // similarity


Node *new_node()
{
    Node *node = malloc(sizeof(Node));
    node->index = malloc(n_mails * sizeof(bool));
    node->child = malloc(CHAR_SIZE * sizeof(Node *));
    node->txt = malloc(CHAR_SIZE * sizeof(int));
    node->tid = -1;
    return node;
}

Lnode* new_list(int data) {
    Lnode* lnode = malloc(sizeof(Lnode));
    lnode->data = data;
    lnode->next = NULL;
    return lnode;
}

void insert_back(Linklist* linklist, int data) {
    Lnode* lnode = new_list(data);
    lnode->next = linklist->back;
    linklist->back = lnode;
}

void insert_front(Linklist* linklist, int data) {
    Lnode* lnode = new_list(data);
    if (linklist->back == NULL) {
        linklist->back = lnode;
        linklist->front = lnode;
        return;
    }

    linklist->front->next = lnode;
    linklist->front = lnode;
}

void free_node(Node *node)
{
    free(node);
}

int c2i(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'z')
        return c - 'a' + 10;
    else if ('A' <= c && c <= 'Z')
        return c - 'A' + 10;
    return -1;
}

char i2c(int i)
{
    if (0 <= i && i <= 9)
        return i + '0';
    else if (10 <= i && i <= 35)
        return i - 10 + 'a';
    return '!';
}

// print tree
void printree(Node *cur, char *str, int level)
{
    for (int id = 0; id < 10000; id++)
    {
        if (cur->index[id] == 1)
        {
            str[level] = '\0';
            printf("%s\n", str);
        }
    }

    for (int i = 0; i < 36; i++)
    {
        if (cur->child[i] != NULL)
        {
            // printf("%d ", cur->child[i]->txt);
            // printf("%c", i2c(cur->child[i]->txt));
            str[level] = i2c(cur->child[i]->txt);
            printree(cur->child[i], str, level + 1);
        }
    }
}

// single-tree dict version
// Build a dict of mails
void st_dict(int id, char text)
{
    int t = c2i(text);
    // printf("%d\n", t);
    // printf("%c\n", i2c(t));
    if (t >= 0)
    {
        if (current->child[t] != NULL)
        {
            current = current->child[t];
        }
        else
        {
            Node *nw;
            nw = new_node();
            nw->txt = t; // for print tree
            current->child[t] = nw;
            current = nw;
        }
    }
    else
    {
        // counting total number of kinds of tokens 
        // for(int i=0; i<n_mails; i++){
        //     if(current->index[i]!=0){
        //         break;
        //     }
        //     if(i==n_mails-1){
        //         ++token_id;
        //     }
        // }
        if(current==sroot)
            return;
        if(current->tid == -1)
            current->tid = ++token_id;
        
        // kot table & t2m table & m2t table building
        if(current->index[id]==0){
            kot[id] = ++token_cnt;
            insert_front(t2m[current->tid], id);
            insert_front(m2t[id], current->tid);
        }
        
        // mark the token
        current->index[id] = 1;
        current = sroot;
    }
}
void make_st_dict(mail m)
{
    token_cnt = 0;
    int i = 0;
    current = sroot;
    while (true)
    {
        st_dict(m.id, m.subject[i]);
        if (m.subject[i] == '\0')
            break;
        i++;
    }
    i = 0;
    current = sroot;
    while (true)
    {
        st_dict(m.id, m.content[i]);
        if (m.content[i] == '\0')
            break;
        i++;
    }
    // for(int i=0; i<10000; i++){
    //     dict(m->id, m->content[i]);
    // }
}

// task 2 function
// find similar
int *s_ans;
void similarity(int mid)
{
    for(int i=0; i<n_mails; i++){
        double AandB = sim_val[i];
        // printf("A and B = %f\n", AandB);
        // printf("A = %d\n", kot[mid]);
        // printf("B = %d\n", kot[i]);
        sim_val[i] = kot[i] + kot[mid] - sim_val[i];
        // printf("%f\n", sim_val[i]);
        sim_val[i] = AandB / sim_val[i];
    }
}

// find token by m2t table
void find_token(int mid)
{
    Lnode *findA;
    findA = m2t[mid]->back;
    while(findA != NULL)
    {
        // find t2m table
        Lnode *findB;
        findB = t2m[findA->data]->back;
        while(findB != NULL){
            sim_val[findB->data] += 1.0;
            findB = findB->next;
        }

        findA = findA->next;
    }
}

void find_similarity(int mid)
{
    // step0
    for(int i=0; i<n_mails; i++){
        sim_val[i] = 0.0;
    }
    // step1: find A(mid) tokens
    find_token(mid);

    // step2: 
    similarity(mid);

    // step3:
    // doing in main()
}


// sort query by mid
void copy_query_data(query *q1, query *q2) {
    q1 = q2;
}

void merge_query(query *qlist, int low, int mid, int high) {
    int left = low;
    int right = mid + 1;
    int combine = low;

    while (left <= mid && right <= high) {
        if (qlist[left].data.find_similar_data.mid < qlist[right].data.find_similar_data.mid) {
            // tmp_qlist[combine] = qlist[left];
            copy_query_data(&tmp_qlist[combine], &qlist[left]);
            left++;
        } else {
            // tmp_qlist[combine] = qlist[right];
            copy_query_data(&tmp_qlist[combine], &qlist[right]);
            right++;
        }
        combine++;
    }
    while (left <= mid) {
        copy_query_data(&tmp_qlist[combine], &qlist[left]);
        // tmp_qlist[combine] = qlist[left];
        left++;
        combine++;
    }
    while (right <= high) {
        copy_query_data(&tmp_qlist[combine], &qlist[right]);
        // tmp_qlist[combine] = qlist[right];
        right++;
        combine++;
    }

    for (int i = low; i <= high; i++) {
        // qlist[i] = tmp_qlist[i];
        copy_query_data(&qlist[i], &tmp_qlist[i]);
    }
}

void sort_query(query *qlist, int low, int high) {
    if (low >= high)
        return;

    int mid = (low + high) / 2;
    sort_query(qlist, low, mid);
    sort_query(qlist, mid + 1, high);
    merge_query(qlist, low, mid, high);
}

int main(void)
{
    api.init(&n_mails, &n_queries, &mails, &queries);

    // table initialize
    token_id = 0;
    kot = malloc(n_mails*sizeof(int));
    t2m = malloc(138100 * sizeof(Linklist*)); // 138079 tokens
    m2t = malloc(n_mails*sizeof(Linklist*));
    for(int i=0; i<138100; i++){
        t2m[i] = malloc(sizeof(Linklist));
    }
    for(int i=0; i<n_mails; i++){
        m2t[i] = malloc(sizeof(Linklist));
    }
    sim_val = malloc(n_mails*sizeof(double));

    // similarity initialize
    s_ans = malloc(n_mails * sizeof(int));

    // single-tree version
    sroot = new_node();
    for (int i = 0; i < n_mails; i++)
        make_st_dict(mails[i]);


    // check queries
    // printf("Total number of kinds of token = %d\n", token_id);

    // sort queries
    int q_cnt = 0;
    for (int q = 0; q < n_queries; q++) {
        if (queries[q].type == find_similar)
            qlist[q_cnt++] = queries[q];
    }
    sort_query(qlist, 0, q_cnt - 1);
    // printf("qcnt = %d\n", q_cnt);

    
    // n_queries
    for (int i = 0; i <q_cnt; i++) {
        find_similarity(qlist[i].data.find_similar_data.mid);
        double threshold = qlist[i].data.find_similar_data.threshold;
        int cnt = 0;
        for(int j=0; j<n_mails; j++){
            if(sim_val[j]>threshold && sim_val[j]<1){
                s_ans[cnt++] = j;
            }
        }
        // for(int j=0; j<20; j++){
        //     printf("%f ", sim_val[j]);
        // }
        // for(int j=0; j<cnt; j++){
        //     printf("%d ", s_ans[j]);
        // }
        // printf("here\n");
        api.answer(qlist[i].id, s_ans, cnt);
    }

    

    return 0;
}

