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
#define BLOCK_WIDTH 50

typedef struct stack {
    int stk[10000];
    int size;
} Stack;

typedef struct node {
    unsigned long *block;
    Stack *mail_stk;
    struct node **child;
    int cnt;
} Node;

typedef struct nstack {
    Node *stk[4096];
    int size;
} NStack;

// define root & current node
Node *root;
Node *current;

unsigned long *pow_2;
int *shift;
int n_blocks;

NStack **token_stk;
double *sim;
int curr_mid = -1;

query *qlist, *tmp_qlist;

NStack *new_nstack() {
    NStack *nstack = malloc(sizeof(NStack));
    nstack->size = 0;
    return nstack;
}

void push_node(NStack *s, Node *token) {
    s->stk[s->size++] = token;
}

Node *pop_node(NStack *s) {
    return (s->size == 0) ? NULL : s->stk[--s->size];
}

Node *top_node(NStack *s) {
    return (s->size == 0) ? NULL : s->stk[s->size - 1];
}

int size(NStack *s) {
    return s->size;
}

void print_nstack(char *msg, NStack *s) {
    printf("%s: ", msg);
    for (int i = 0; i < s->size; i++)
        printf("%c ", s->stk[i]);
    printf("\n");
}

Stack *new_stack() {
    Stack *stack = malloc(sizeof(Stack));
    stack->size = 0;
    return stack;
}

void push(Stack *s, int data) {
    s->stk[s->size++] = data;
}

int pop(Stack *s) {
    return (s->size == 0) ? NULL : s->stk[--s->size];
}

int top(Stack *s) {
    return (s->size == 0) ? NULL : s->stk[s->size - 1];
}

bool not_empty(Stack *s) {
    return s->size > 0;
}

void print_stack(char *msg, Stack *s) {
    printf("%s: ", msg);
    for (int i = 0; i < s->size; i++)
        printf("%d ", s->stk[i]);
    printf("\n");
}

Node *new_node() {
    Node *node = malloc(sizeof(Node));
    node->block = malloc(n_blocks * sizeof(unsigned long));
    node->child = malloc(CHAR_SIZE * sizeof(Node *));
    return node;
}

inline int c2i(char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'z')
        return c - 'a' + 10;
    else if ('A' <= c && c <= 'Z')
        return c - 'A' + 10;
    return -1;
}

inline char i2c(int i) {
    if (0 <= i && i <= 9)
        return i + '0';
    else if (10 <= i && i <= 35)
        return i - 10 + 'a';
    return '!';
}

// print tree
void printree(Node *cur, char *str, int level) {
    int cnt = 0;
    for (int i = 0; i < n_blocks; i++) {
        for (int j = 0; j < BLOCK_WIDTH; j++) {
            if (cur->block[i] & pow_2[j])
                cnt++;
        }
    }

    if (cnt) {
        str[level] = '\0';
        printf("%-*s\t%d ", 20, str, cnt);
        for (int i = 0; i < cur->mail_stk->size; i++)
            printf("%d ", cur->mail_stk->stk[i]);
        printf("\n");
    }

    for (int i = 0; i < 36; i++) {
        if (cur->child[i] != NULL) {
            str[level] = i2c(i);
            printree(cur->child[i], str, level + 1);
        }
    }
}

// Build a dict of a mail
inline void dict(int id, int bid, int rem, char text) {
    int t = c2i(text);
    // printf("%c\n", text);
    if (t >= 0) {
        // printf("%c", text);
        if (current->child[t] == NULL)
            current->child[t] = new_node();
        current = current->child[t];
    } else {
        if (current == root)
            return;
        // printf("\n");
        if (!(current->block[bid] & pow_2[rem])) {
            // printf("added %lld in index %d\n", pow_2[rem], bid);
            current->block[bid] |= pow_2[rem];
            if (current->cnt == 0) {
                current->mail_stk = new_stack();
            }
            current->cnt++;
            push(current->mail_stk, id);
            push_node(token_stk[id], current);
        }
        current = root;
    }
}

inline void make_dict(mail m) {
    int bid = m.id / BLOCK_WIDTH, rem = m.id % BLOCK_WIDTH;
    int i = 0;
    current = root;
    while (m.subject[i] != '\0')
        dict(m.id, bid, rem, m.subject[i++]);
    dict(m.id, bid, rem, m.subject[i]);
    i = 0;
    current = root;
    while (m.content[i] != '\0')
        dict(m.id, bid, rem, m.content[i++]);
    dict(m.id, bid, rem, m.content[i]);
}

int *f_ans;
int f_cnt;

void query_similar(int mid, double threshold) {
    if (curr_mid == mid) {
        for (int i = 0; i < mid; i++) {
            if (sim[i] > threshold)
                f_ans[f_cnt++] = i;
        }
        for (int i = mid + 1; i < n_mails; i++) {
            if (sim[i] > threshold)
                f_ans[f_cnt++] = i;
        }
    } else {
        for (int i = 0; i < n_mails; i++)
            sim[i] = 0.0;

        int size = token_stk[mid]->size;
        Node **token = token_stk[mid]->stk;
        for (int t = 0; t < size; t++) {
            Stack *s = token[t]->mail_stk;
            for (int i = 0; i < s->size; i++) {
                sim[s->stk[i]] += 1.0;
            }
        }
        for (int i = 0; i < mid; i++) {
            sim[i] = sim[i] / (token_stk[mid]->size + token_stk[i]->size - sim[i]);
            if (sim[i] > threshold)
                f_ans[f_cnt++] = i;
        }
        for (int i = mid + 1; i < n_mails; i++) {
            sim[i] = sim[i] / (token_stk[mid]->size + token_stk[i]->size - sim[i]);
            if (sim[i] > threshold)
                f_ans[f_cnt++] = i;
        }
        curr_mid = mid;
    }
}

void initialize() {
    n_blocks = n_mails / BLOCK_WIDTH;

    unsigned long num = 1;
    pow_2 = malloc(BLOCK_WIDTH * sizeof(unsigned long));
    for (int i = 0; i < BLOCK_WIDTH; i++) {
        pow_2[i] = num;
        num = num << 1;
    }
    shift = malloc(n_blocks * sizeof(int));
    for (int i = 0; i < n_blocks; i++)
        shift[i] = i * BLOCK_WIDTH;

    root = new_node();
    current = root;

    f_ans = malloc(n_mails * sizeof(int));

    sim = malloc(n_mails * sizeof(double));
    token_stk = malloc(n_mails * sizeof(NStack *));
    for (int i = 0; i < n_mails; i++) {
        token_stk[i] = new_nstack();
    }

    qlist = malloc(n_queries * sizeof(query));
    tmp_qlist = malloc(n_queries * sizeof(query));
}

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

// #include <sys/resource.h>

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);

    // clock_t begin = clock();
    // struct rusage *memory = malloc(sizeof(struct rusage));

    initialize();

    // read the mail & build the dictionary
    for (int i = 0; i < n_mails; i++)
        make_dict(mails[i]);

    // char *str = malloc(2048 * sizeof(char));
    // printree(root, str, 0);
    // return 0;

    // sort queries
    int q_cnt = 0;
    for (int q = 0; q < n_queries; q++) {
        if (queries[q].type == find_similar)
            qlist[q_cnt++] = queries[q];
    }
    sort_query(qlist, 0, q_cnt - 1);

    // for (int q = 0; q < n_queries; q++) {
    //     if (queries[q].type == find_similar)
    //         printf("%d %d\n", queries[q].id, queries[q].data.find_similar_data.mid);
    // }

    // check queries
    for (int q = 0; q < q_cnt; q++) {
        f_cnt = 0;
        query_similar(qlist[q].data.find_similar_data.mid, qlist[q].data.find_similar_data.threshold);
        api.answer(qlist[q].id, f_ans, f_cnt);

        // printf("I answered id %d\n", qlist[q].id);
        // for (int i = 0; i < f_cnt; i++)
        //     printf("%d ", f_ans[i]);
        // printf("\n");
        // printf("%d\n\n\n", f_cnt);
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
