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

// unused
int *outA;
bool bool_r, bool_a, bool_b;
int m_level;

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
mNode **root; // multi-tree root
Node *sroot; // single-tree root
Node *current;
Node *current2;
mNode *currentm;
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

mNode *new_mnode()
{
    mNode *mnode = malloc(sizeof(mNode));
    mnode->index = false;
    mnode->child = malloc(CHAR_SIZE * sizeof(mNode *));
    mnode->txt = malloc(CHAR_SIZE * sizeof(int));
    mnode->tid = -1;
    return mnode;
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



// multi-tree dict version
// Build a dict of a mail
void mt_dict(int id, char text)
{
    int t = c2i(text);
    // printf("%d\n", t);
    // printf("%c\n", i2c(t));
    if (t >= 0)
    {
        if (currentm->child[t] != NULL)
        {
            currentm = currentm->child[t];
        }
        else
        {
            mNode *nw;
            nw = new_mnode();
            nw->txt = t; // for print tree
            currentm->child[t] = nw;
            currentm = nw;
        }
        current = current->child[t];
    }
    else
    {
        if(currentm==root[id])
            return;
        // kot table building
        if(currentm->index==0){
            kot[id] = ++token_cnt;
            insert_front(t2m[current->tid], id);
        }
        // give multi-tree the id of token in t2m table
        currentm->tid = current->tid;

        currentm->index = 1;
        currentm = root[id];
        current = sroot;
    }
}
void make_mt_dict(mail m)
{
    token_cnt = 0;
    int i = 0;
    currentm = root[m.id];
    current = sroot;
    while (true)
    {
        mt_dict(m.id, m.subject[i]);
        if (m.subject[i] == '\0')
            break;
        i++;
    }
    i = 0;
    currentm = root[m.id];
    current = sroot;
    while (true)
    {
        mt_dict(m.id, m.content[i]);
        if (m.content[i] == '\0')
            break;
        i++;
    }
    // for(int i=0; i<10000; i++){
    //     dict(m->id, m->content[i]);
    // }
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
        
        
        
        
        // mark the token
        current->index[id] = 1;
        current = sroot;
    }
}
void make_st_dict(mail m)
{
    
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

// Matching
bool matching(char q, int level)
{
    if (current->child[c2i(q)] != NULL)
    {
        current = current->child[c2i(q)];
    }
    else
    {
        current = root;
    }
}
void match(char q)
{
    // if(!strcmp(q, "["))
    // if(!strcmp(q, "("))
    // if(!strcmp(q, ")"))
    // if(!strcmp(q, "!"))
    switch (q)
    {
    case '[':
        m_level -= 1;
        break;

    case ']':
        m_level += 1;
        break;

    case '(':
        m_level -= 1;
        break;

    case ')':
        m_level += 1;
        break;

    case '!':
        m_level -= 1;
        break;

    case '&':
        if (bool_a == false)
            break;

    case '|':
        if (bool_a == true)
            break;

    default:
        matching(q, m_level);
        ;
    }
    // while(strcmp(q, "]")){
    //     if(!strcmp(q, current))
    // }
}

// task 1 function
bool *expr_ans;
int *e_ans;


int recur_index = 0;
int recur_invert = 0;

void combine_ans(bool *ans1, bool *ans2, bool op, bool invert) {
    for (int i = 0; i < n_mails; i++) {
        if (op)
            ans1[i] = ans1[i] & ((invert) ? !ans2[i] : ans2[i]);
        else
            ans1[i] = ans1[i] | ((invert) ? !ans2[i] : ans2[i]);
    }
}

void print_ans(bool *ans) {
    int cnt = 0;
    for (int j = 0; j < n_mails; j++) {
        if (ans[j])
            cnt++;
    }
    if (cnt >= n_mails / 2) {
        printf(" [inv]");
        for (int j = 0; j < n_mails; j++) {
            if (!ans[j])
                printf(" (%d)", j);
        }
    } else {
        for (int j = 0; j < n_mails; j++) {
            if (ans[j])
                printf(" (%d)", j);
        }
    }
    printf("\n");
}
// single-tree version query
bool *query_st_expression(char *expr, int start, int level) {
    bool invert = false, and = false;
    bool *ans = malloc(n_mails * sizeof(bool));
    bool *temp_ans = malloc(n_mails * sizeof(bool));
    bool *bracket_ans;

    int curr = start;
    Node *curr_node = sroot;
    // printf("=============== level %-*d start  ===============\n", 2, level);
    while (1) {
        switch (expr[curr]) {
            case '\0':
                recur_invert = invert;
                // printf("=============== level %-*d finish ===============\n", 2, level);
                return ans;
            case ')':
                // TODO: end recursion and return ans at curr_node
                if (curr_node == sroot) {
                    // printf("is root\n");
                    recur_invert = invert;
                    recur_index = curr;
                    // printf("=============== level %-*d finish ===============\n", 2, level);
                    return ans;
                }

                // printf("\n");
                // printf("node_ans =");
                // print_ans(curr_node->index);

                recur_invert = invert;
                recur_index = curr;
                // printf("=============== level %-*d finish ===============\n", 2, level);
                return curr_node->index;
            case '(':
                // TODO: start recursion and calculate ans
                bracket_ans = query_st_expression(expr, curr + 1, level + 1);

                curr = recur_index;
                // printf("next = %c\n", expr[curr + 1]);
                // printf("inv = %d, recurinv = %d, and = %d\n", invert, recur_invert, and);
                if (expr[curr + 1] == '&') {
                    if (and) {
                        combine_ans(temp_ans, bracket_ans, 1, recur_invert);
                    } else {
                        combine_ans(temp_ans, bracket_ans, 0, recur_invert);
                    }
                } else {
                    if (and) {
                        combine_ans(temp_ans, bracket_ans, 1, recur_invert);
                        combine_ans(ans, temp_ans, 0, invert);
                    } else {
                        combine_ans(ans, bracket_ans, 0, recur_invert);
                    }
                }
                if (expr[curr + 1] == '|')
                    and = false;
                recur_invert = 0;

                // printf("     ans =");
                // print_ans(ans);
                // printf("temp_ans =");
                // print_ans(temp_ans);
                break;
            case '!':
                // printf("curr = %c\n", expr[curr]);
                invert = true;
                // printf("inv = %d, recurinv = %d, and = %d\n", invert, recur_invert, and);
                break;
            case '&':
                // printf("curr = %c\n", expr[curr]);
                and = true;
                // printf("inv = %d, recurinv = %d, and = %d\n", invert, recur_invert, and);
                break;
            case '|':
                // printf("curr = %c\n", expr[curr]);
                break;
            default:
                // TODO: go down one level in tree
                // printf("%c", expr[curr]);
                curr_node = curr_node->child[c2i(expr[curr])];
                break;
        }
        curr++;
    }

    return NULL;
}

// multi-tree version query
void combine_mt_ans(bool ans1, bool ans2, bool op, bool invert) {
    if (op)
        ans1 = ans1 & ((invert) ? !ans2 : ans2);
    else
        ans1 = ans1 | ((invert) ? !ans2 : ans2);
}

bool query_mt_expression(char *expr, int start, int level) {
    bool invert = false, and = false;
    bool ans;
    bool temp_ans;
    bool bracket_ans;
    bool wrong_match=0;

    int curr = start;
    Node *curr_node = root;
    // printf("=============== level %-*d start  ===============\n", 2, level);
    while (1) {
        switch (expr[curr]) {
            case '\0':
                recur_invert = invert;
                // printf("=============== level %-*d finish ===============\n", 2, level);
                return ans;
            case ')':
                // TODO: end recursion and return ans at curr_node
                if (curr_node == root) {
                    // printf("is root\n");
                    recur_invert = invert;
                    recur_index = curr;
                    // printf("=============== level %-*d finish ===============\n", 2, level);
                    return ans;
                }
                
                // printf("\n");
                // printf("node_ans =");
                // print_ans(curr_node->index);

                recur_invert = invert;
                recur_index = curr;

                // printf("=============== level %-*d finish ===============\n", 2, level);
                if(wrong_match)
                    return false;
                return curr_node->index;
            case '(':
                // TODO: start recursion and calculate ans
                bracket_ans = query_mt_expression(expr, curr + 1, level + 1);

                curr = recur_index;
                // printf("next = %c\n", expr[curr + 1]);
                // printf("inv = %d, recurinv = %d, and = %d\n", invert, recur_invert, and);
                if (expr[curr + 1] == '&') {
                    if (and) {
                        combine_mt_ans(temp_ans, bracket_ans, 1, recur_invert);
                    } else {
                        combine_mt_ans(temp_ans, bracket_ans, 0, recur_invert);
                    }
                } else {
                    if (and) {
                        combine_mt_ans(temp_ans, bracket_ans, 1, recur_invert);
                        combine_mt_ans(ans, temp_ans, 0, invert);
                    } else {
                        combine_mt_ans(ans, bracket_ans, 0, recur_invert);
                    }
                }
                if (expr[curr + 1] == '|')
                    and = false;
                recur_invert = 0;

                // printf("     ans =");
                // print_ans(ans);
                // printf("temp_ans =");
                // print_ans(temp_ans);
                break;
            case '!':
                // printf("curr = %c\n", expr[curr]);
                invert = true;
                // printf("inv = %d, recurinv = %d, and = %d\n", invert, recur_invert, and);
                break;
            case '&':
                // printf("curr = %c\n", expr[curr]);
                and = true;
                // printf("inv = %d, recurinv = %d, and = %d\n", invert, recur_invert, and);
                break;
            case '|':
                // printf("curr = %c\n", expr[curr]);
                break;
            default:
                // TODO: go down one level in tree
                // printf("%c", expr[curr]);
                if(!wrong_match){
                    if(curr_node->child[c2i(expr[curr])] != NULL)
                        curr_node = curr_node->child[c2i(expr[curr])];
                    else
                        wrong_match = 1;
                }
                break;
        }
        curr++;
    }

    return NULL;
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
void find_token(mNode *cur)
{
    if (cur->index == 1)
    {
        // find t2m table
        Lnode *findB;
        findB = t2m[cur->tid]->back;
        while(findB != NULL){
            sim_val[findB->data] += 1;
            findB = findB->next;
        }
    }
    for (int i = 0; i < 36; i++)
    {
        if (cur->child[i] != NULL)
        {
            // printf("%d ", cur->child[i]->txt);
            // printf("%c", i2c(cur->child[i]->txt));
            find_token(cur->child[i]);
        }
    }
}
void find_similarity(int mid)
{
    // step0
    for(int i=0; i<n_mails; i++){
        sim_val[i] = 0.0;
    }
    // step1: find A(mid) tokens
    currentm = root[mid];
    find_token(currentm);

    // step2: 
    similarity(mid);

    // step3:
    // doing in main()
}


int main(void)
{
    api.init(&n_mails, &n_queries, &mails, &queries);
    // printf("%d\n", n_mails);
    // printf("%d\n", n_queries);
    // printf("%s\n", &mails[0]);
    // current = root;

    // tree initialize
    root = malloc(n_mails*sizeof(mNode*));

    // table initialize
    token_id = 0;
    kot = malloc(n_mails*sizeof(int));
    t2m = malloc(138100 * sizeof(Linklist*)); // 138079 tokens
    for(int i=0; i<138100; i++){
        t2m[i] = malloc(sizeof(Linklist));
    }
    sim_val = malloc(n_mails*sizeof(double));


    // matching initialize
    expr_ans = malloc(n_mails*sizeof(bool));
    e_ans = malloc(n_mails * sizeof(int));

    // similarity initialize
    s_ans = malloc(n_mails * sizeof(int));


    // single-tree version
    sroot = new_node();
    for (int i = 0; i < n_mails; i++)
        make_st_dict(mails[i]);

    // multi-tree version
    // read the mail & build the dictionary
    for (int i = 0; i < n_mails; i++)
    {
        root[i] = new_mnode();
        make_mt_dict(mails[i]);
    }

    // test dict
    char *str = malloc(2048 * sizeof(char));
    // root[0] = new_node();
    // sroot = new_node();
    // make_st_dict(mails[0]);
    // make_mt_dict(mails[0]);
    // printf("%d", mails[0].id);
    // printree(root[0], str, 0);
    // printree(sroot, str, 0);
    // printree(root[0], str, 0);
    // printree(root[1], str, 0);
    // printree(root[2], str, 0);

    
    // check queries
    // for (int i = 0; i < n_queries; i++) {
    //     if (queries[i].type == expression_match) {
    //         expr_ans = query_st_expression(queries[i].data.expression_match_data.expression, 0, 1);
    //         int cnt = 0;
    //         for (int j = 0; j < n_mails; j++) {
    //             if (recur_invert ^ expr_ans[j])
    //                 e_ans[cnt++] = j;
    //         }
    //         // printf("here\n");
    //         api.answer(queries[i].id, e_ans, cnt);
    //     }
    // }

    // printf("Total number of kinds of token = %d\n", token_id);

    // n_queries
    for (int i = 0; i <n_queries; i++) {
        if (queries[i].type == find_similar) {
            find_similarity(queries[i].data.find_similar_data.mid);
            double threshold = queries[i].data.find_similar_data.threshold;
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
            api.answer(queries[i].id, s_ans, cnt);
        }
    }
    
    
    

    // answer the queries
    // for (int i = 0; i < n_queries; i++) {
    //     if (queries[i].type == expression_match) {
    //         api.answer(queries[i].id, NULL, 0);
    //     }
    // }

    // for (int i = 0; i < n_queries; i++) {
    //     if (queries[i].type == group_analyse) {
    //         api.answer(queries[i].id, NULL, 0);
    //     }
    // }
    free(str);

    return 0;
}
/*
for (int i = 0; i < n_queries; i++)
    {
        if (queries[i].type == expression_match)
        {
            bool_b = true;
            // executing
            char *expr = queries[i].data.expression_match_data.expression;
            expr_ans = query_st_expression(expr, 0, 1);
            // expr_ans = query_mt_expression(expr, 0, 1);
            // answer
            int cnt = 0;
            for (int j = 0; j < n_mails; j++) {
                if (recur_invert ^ expr_ans[j])
                    e_ans[cnt++] = j;
            }
            api.answer(queries[i].id, e_ans, cnt);
        }
    }
*/
