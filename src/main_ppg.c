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

#define CHAR_SIZE 36

typedef struct node {
    bool *index;
    struct node **child;
    int *txt;
} Node;

// define root & current node
Node *root;
Node *current;

Node *new_node() {
    Node *node = malloc(sizeof(Node));
    node->index = malloc(n_mails * sizeof(bool));
    node->child = malloc(CHAR_SIZE * sizeof(Node *));
    node->txt = malloc(CHAR_SIZE * sizeof(int));
    return node;
}

void free_node(Node *node) {
    free(node);
}

int c2i(char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'z')
        return c - 'a' + 10;
    else if ('A' <= c && c <= 'Z')
        return c - 'A' + 10;
    return -1;
}

char i2c(int i) {
    if (0 <= i && i <= 9)
        return i + '0';
    else if (10 <= i && i <= 35)
        return i - 10 + 'a';
    return '!';
}

// print tree
void printree(Node *cur, char *str, int level) {
    int cnt = 0;
    for (int id = 0; id < 10000; id++)
        cnt += cur->index[id];

    if (cnt) {
        str[level] = '\0';
        printf("%-*s\t%d\n", 20, str, cnt);
    }

    for (int i = 0; i < 36; i++) {
        if (cur->child[i] != NULL) {
            // printf("%d ", cur->child[i]->txt);
            // printf("%c", i2c(cur->child[i]->txt));
            str[level] = i2c(cur->child[i]->txt);
            printree(cur->child[i], str, level + 1);
        }
    }
}

// Build a dict of a mail
void dict(int id, char text) {
    int t = c2i(text);
    // printf("%c ", text);
    // printf("%d\n", t);
    if (t >= 0) {
        if (current->child[t] != NULL) {
            current = current->child[t];
        } else {
            Node *nw;
            nw = new_node();
            nw->txt = t;  // for print tree
            current->child[t] = nw;
            current = nw;
        }
    } else {
        current->index[id] = 1;
        current = root;
    }
}

void make_dict(mail m) {
    int i = 0;
    current = root;
    while (true) {
        dict(m.id, m.subject[i]);
        if (m.subject[i] == '\0')
            break;
        i++;
    }
    i = 0;
    current = root;
    while (true) {
        dict(m.id, m.content[i]);
        if (m.content[i] == '\0')
            break;
        i++;
    }
}

// TODO: malloc(n_mails*sizeof(bool)) in main
// TODO: set all bits to 0 in main before every query
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

bool *query_expression(char *expr, int start, int level) {
    bool invert = false, and = false;
    bool *ans = malloc(n_mails * sizeof(bool));
    bool *temp_ans = malloc(n_mails * sizeof(bool));
    bool *bracket_ans;

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
                return curr_node->index;
            case '(':
                // TODO: start recursion and calculate ans
                bracket_ans = query_expression(expr, curr + 1, level + 1);

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

// #include <sys/resource.h>

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);

    // clock_t begin = clock();
    // struct rusage *memory = malloc(sizeof(struct rusage));

    root = new_node();
    current = root;

    expr_ans = malloc(n_mails * sizeof(bool));
    e_ans = malloc(n_mails * sizeof(int));

    // read the mail & build the dictionary
    for (int i = 0; i < n_mails; i++)
        make_dict(mails[i]);

    // char *str = malloc(2048 * sizeof(char));
    // printree(root, str, 0);

    // check queries
    for (int i = 0; i < n_queries; i++) {
        if (queries[i].type == expression_match) {
            expr_ans = query_expression(queries[i].data.expression_match_data.expression, 0, 1);
            int cnt = 0;
            for (int j = 0; j < n_mails; j++) {
                if (recur_invert ^ expr_ans[j])
                    e_ans[cnt++] = j;
            }
            api.answer(queries[i].id, e_ans, cnt);

            // if (i > 5230) {
            //     printf("i answered id %d\n", queries[i].id);
            //     for (int j = 0; j < cnt; j++) {
            //         printf("%d ", e_ans[j]);
            //     }
            //     printf("\n");
            //     printf("%d\n\n\n", cnt);
            // }
            // printf("i answered id %d\n", queries[i].id);
            // for (int j = 0; j < cnt; j++) {
            //     printf("%d ", e_ans[j]);
            // }
            // printf("\n");
            // printf("%d\n\n\n", cnt);
        }
    }

    // clock_t end = clock();
    // getrusage(RUSAGE_SELF, memory);
    // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    // long memory_use = memory->ru_maxrss;
    // printf("Total time: %f s\n", time_spent);
    // printf("Total memory: %ld MB\n", memory_use / 1024);
    // free(memory);

    return 0;
}
