#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
using namespace std;

#define NC "\033[0m"
#define LR "\033[1;31m"
#define LG "\033[1;32m"

typedef unsigned long long ull;
typedef pair<ull, double> P;

int main() {
    ifstream ans("testdata/test.out", fstream::in);

    int query_id;
    double reward;
    ull digest;

    map<int, P> answers;

    while (ans >> query_id >> reward >> hex >> digest)
        answers[query_id] = P(digest, reward);

    int ans_cnt = 0;

    int w_cnt = 0;
    double pos_score = 0.0, neg_score = 0.0;

    while (cin >> query_id >> hex >> digest) {
        if (answers.find(query_id) == answers.end())
            continue;

        if (answers[query_id].first == digest) {
            pos_score += answers[query_id].second;
        } else {
            neg_score -= answers[query_id].second * 0.5;
            w_cnt++;
        }

        answers[query_id] = P(0, 0);
        ans_cnt++;
    }

    cout << endl;
    cout << "Result:" << endl;
    cout << "Answered " << ans_cnt << " times, ";
    if (w_cnt == 0) {
        cout << LG << "all passed" << NC << endl;
    } else {
        cout << LR << "total " << w_cnt << " wrong answers" << NC << endl;
    }
    printf("Original |%9.2f\n", pos_score);
    printf("Penalty  |%9.2f\n", neg_score);
    printf("Final    |%9.2f\n", pos_score + neg_score);

    return 0;
}