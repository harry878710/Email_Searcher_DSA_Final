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
int cnt;
int *outA;
bool bool_r, bool_a, bool_b;
int m_level;

#define CHAR_SIZE 36

typedef struct node
{
    bool *index;
    struct node **child;
    int *txt;
} Node;

// define root & current node
Node *root;
Node *current;

Node *new_node()
{
    Node *node = malloc(sizeof(Node));
    node->index = malloc(n_mails * sizeof(bool));
    node->child = malloc(CHAR_SIZE * sizeof(Node *));
    node->txt = malloc(CHAR_SIZE * sizeof(int));
    return node;
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
            // printf("%s\n", str);
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

// Build a dict of a mail
void dict(int id, char text)
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
        current->index[id] = 1;
        current = root;
    }
}

void make_dict(mail m)
{
    int i = 0;
    current = root;
    while (true)
    {
        dict(m.id, m.subject[i]);
        if (m.subject[i] == '\0')
            break;
        i++;
    }
    i = 0;
    current = root;
    while (true)
    {
        dict(m.id, m.content[i]);
        if (m.content[i] == '\0')
            break;
        i++;
    }
    // for(int i=0; i<10000; i++){
    //     dict(m->id, m->content[i]);
    // }
}

// TODO: malloc(n_mails*sizeof(bool)) in main
// TODO: set all bits to 0 in main before every query
bool *expr_ans;
int bracket_index=0;
bool tmp_ans;

bool *query_expression(char *expr, int start)
{
    bool invert = false;
    bool op = false; // false: or, true: and
    
    int curr = start;
    Node *curr_node = root;
    while (1)
    {
        switch (expr[curr])
        {
        case '\0':
            return;
        case ')':
            // TODO: end recursion and return ans at curr_node
            bracket_index--;
            // tmp_ans = 
            break;
        case '(':
            // TODO: start recursion and calculate ans
            bracket_index++;
            tmp_ans = *query_expression(*expr, start);
            break;
        case '!':
            invert = true;
            break;
        case '&':
            op = true;
            break;
        case '|':
            op = false;
            break;
        default:
            // TODO: go down one level in tree
            curr++;
            break;
        }
    }

    return NULL;
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

int main(void)
{
    api.init(&n_mails, &n_queries, &mails, &queries);

    // printf("%d\n", n_mails);
    // printf("%d\n", n_queries);
    // printf("%s\n", &mails[0]);
    root = new_node();
    current = root;
    char *str = malloc(2048 * sizeof(char));

    // matching
    expr_ans = malloc(n_mails*sizeof(bool));

    // read the mail & build the dictionary
    for (int i = 0; i < n_mails; i++)
    {
        make_dict(mails[i]);
    }
    // make_dict(mails[0]);
    // printree(root, str, 0);

    // check queries
    for (int i = 0; i < n_queries; i++)
    {
        if (queries[i].type == expression_match)
        {
            cnt = 0;
            bool_b = true;
            // executing
            char *expr = queries[i].data.expression_match_data.expression;
            expr_ans[i] = query_expression(expr, 0);
        }
        // answer
        api.answer(queries[i].id, outA, cnt);
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
