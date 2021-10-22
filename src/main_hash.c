#include "api.h"
// api.h must be the first line

// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) a > b ? a : b

#define CHAR_SIZE 36

int n_mails, n_queries;
mail *mails;
query *queries;

// ********************************************************************************
// Expression Match Section
// ********************************************************************************

#define MAX_TOKEN 100000

typedef struct token {
    unsigned short serial[6];
} Token;

typedef struct table {
    Token *table[MAX_TOKEN];
} Table;

Table *table;

Token *new_node(int *serial) {
    Token *token = malloc(sizeof(Token));
    for (int i = 0; i < 6; i++)
        token->serial[i] = serial[i];
    return token;
}

void free_node(Token *node) {
    free(node);
}

int c2i(const char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'z')
        return c - 'a' + 10;
    else if ('A' <= c && c <= 'Z')
        return c - 'A' + 10;
    return -1;
}

void insert_tokens(mail m) {
    int i = 0, pow = 1;
    int sum = 0;
    int cnt = 0;
    unsigned short serial[6];
    for (int i = 0; i < 6; i++)
        serial[i] = -1;
    char *s = malloc(2048 * sizeof(char));

    while (true) {
        int code = c2i(m.content[i]);
        if (code < 0) {
            if (cnt % 3 != 0)
                serial[cnt / 3] = sum;

            s[cnt] = '\0';
            printf("%*s: ", 18, s);
            for (int i = 0; i < 6; i++)
                printf("%*d ", 5, serial[i]);
            printf("\n\n");

            cnt = 0;
            pow = 1;
            for (int i = 0; i < 6; i++)
                serial[i] = -1;
        } else {
            s[cnt] = m.content[i];
            sum += pow * code;
            cnt++;
        }

        if (m.content[i] == '\0')
            break;

        if (cnt % 3 == 0 && cnt <= 18) {  // TODO: char size > 18
            serial[cnt / 3 - 1] = sum;
            sum = 0;
            pow = 1;
        } else {
            pow *= CHAR_SIZE;
        }
        i++;
    }

    free(s);
}

// ********************************************************************************
// Group Analyse Section
// ********************************************************************************

typedef struct set {
    int parent;
    int size;
} Set;

int g_ans[2];

char name[100000][32];
int set[100000];
int set_index[1024];
Set *mail_set;
int set_cnt = 0;

// int c2i(const char c) {
//     if ('0' <= c && c <= '9')
//         return c - '0';
//     else if ('a' <= c && c <= 'z')
//         return c - 'a' + 10;
//     else if ('A' <= c && c <= 'Z')
//         return c - 'A' + 10;
//     return -1;
// }

void i2s(int num, char *c) {
    int len = 1, pow = 1;
    int temp = num;
    temp /= 10;
    while (temp) {
        len++;
        pow *= 10;
        temp /= 10;
    }

    int i = 0;
    while (len--) {
        c[i++] = (num / pow) + '0';
        num = num % pow;
        pow /= 10;
    }
    c[i] = '\0';
}

int hash(const char *s) {
    int sum = 0;
    int i = 0, pow = 1;
    while (s[i] != '\0') {
        sum = (sum + pow * c2i(s[i++])) % 100000;
        pow = (pow * CHAR_SIZE) % 100000;
    }
    // printf("%d\n", sum);
    return sum;
}

int make_set(const char *s) {
    int i = hash(s);
    while (set[i] != 0) {
        if (!strcmp(s, name[i]))
            return set[i];
        i++;
    }
    strcpy(name[i], s);
    set[i] = ++set_cnt;
    return set[i];
}

void init_set() {
    int s, index;
    for (int i = 0; i < n_mails; i++) {
        index = make_set(mails[i].from);
        // printf("%*s: %d\n", 12, mails[i].from, index);
        i2s(index, mails[i].from);
        index = make_set(mails[i].to);
        // printf("%*s: %d\n", 12, mails[i].to, index);
        i2s(index, mails[i].to);
    }
}

void reset_set(int i) {
    mail_set[i].parent = i;
    mail_set[i].size = 1;
}

int find_set(int i) {
    int curr = i;
    while (mail_set[curr].parent != curr)
        curr = mail_set[curr].parent;
    mail_set[i].parent = curr;
    return curr;
}

void union_set(int i1, int i2) {
    int s1 = find_set(i1), s2 = find_set(i2);
    // printf("s1 = %d, s2 = %d\n", s1, s2);

    if (s1 == s2)
        return;

    if (mail_set[s1].size >= mail_set[s2].size) {
        mail_set[s2].parent = s1;

        if (mail_set[s1].size == 1)
            g_ans[0]++;
        else if (mail_set[s2].size > 1)
            g_ans[0]--;
        mail_set[s1].size += mail_set[s2].size;
        g_ans[1] = MAX(g_ans[1], mail_set[s1].size);
    } else {
        mail_set[s1].parent = s2;

        if (mail_set[s2].size == 1)
            g_ans[0]++;
        else if (mail_set[s1].size > 1)
            g_ans[0]--;
        mail_set[s2].size += mail_set[s1].size;
        g_ans[1] = MAX(g_ans[1], mail_set[s2].size);
    }
}

void group(int len, int *mids) {
    int id;
    int size = 2 * len;
    g_ans[0] = 0, g_ans[1] = 0;

    int cnt = 0;
    for (int i = 0; i < len; i++) {
        id = mids[i];
        set_index[cnt] = atoi(mails[id].from);
        reset_set(set_index[cnt++]);
        set_index[cnt] = atoi(mails[id].to);
        reset_set(set_index[cnt++]);
    }

    for (int i = 0; i < size;) {
        // printf("from: %d, to: %d\n", index[i], index[i+1]);
        union_set(set_index[i++], set_index[i++]);
        // printf("%d %d\n", g_ans[0], g_ans[1]);
    }
}

#include <sys/resource.h>

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);

    clock_t begin = clock();
    struct rusage *memory = malloc(sizeof(struct rusage));

    insert_tokens(mails[0]);

    // init_set();
    // mail_set = malloc(set_cnt * sizeof(Set));

    // for (int i = 0; i < n_queries; i++) {
    //     if (queries[i].type == group_analyse) {
    //         group(queries[i].data.group_analyse_data.len, queries[i].data.group_analyse_data.mids);
    //         // printf("%d %d\n", g_ans[0], g_ans[1]);
    //         api.answer(queries[i].id, g_ans, 2);
    //     }
    // }

    // free(mail_set);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    getrusage(RUSAGE_SELF, memory);

    printf("Total time: %f s\n", time_spent);
    printf("Total memory: %ld KB\n", memory->ru_maxrss);
    free(memory);
    return 0;
}
