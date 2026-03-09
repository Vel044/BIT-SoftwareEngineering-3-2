#include <bits/stdc++.h>
using namespace std;

// 解析逗号分隔的磁道请求序列
vector<int> parse_sequence(const string &s) {
    vector<int> seq;
    string num;
    for (char c : s) {
        if (c == ',') {
            if (!num.empty()) {
                seq.push_back(stoi(num));
                num.clear();
            }
        } else if (!isspace(c)) {
            num.push_back(c);
        }
    }
    if (!num.empty()) {
        seq.push_back(stoi(num));
    }
    return seq;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int algo;           // 1=FCFS, 2=SSTF, 3=SCAN, 4=C-SCAN
    int headPos;        // 当前磁头位置
    int direction;      // 1=向号增大方向, 0=向号减小方向
    string seqLine;

    // 读入
    if (!(cin >> algo)) return 0;
    cin >> headPos;
    cin >> direction;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, seqLine);

    vector<int> requests = parse_sequence(seqLine);
    int n = requests.size();

    vector<int> order;  // 最终的磁头访问顺序 (包含起始 headPos)
    order.reserve(n + 1);
    order.push_back(headPos);

    if (algo == 1) {
        // 1. FCFS（先来先服务）
        for (int r : requests) {
            order.push_back(r);
        }

    } else if (algo == 2) {
        // 2. SSTF（最短寻道时间优先）
        vector<bool> used(n, false);
        int curr = headPos;
        for (int k = 0; k < n; k++) {
            int bestIdx = -1;
            int bestDist = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!used[i]) {
                    int d = abs(requests[i] - curr);
                    if (d < bestDist) {
                        bestDist = d;
                        bestIdx = i;
                    }
                }
            }
            used[bestIdx] = true;
            curr = requests[bestIdx];
            order.push_back(curr);
        }

    } else if (algo == 3 || algo == 4) {
        // 3. SCAN 或 4. C-SCAN
        // 先对请求做排序
        vector<int> reqs = requests;
        sort(reqs.begin(), reqs.end());

        // 把请求分为两部分：小于 headPos 与 大于等于 headPos
        vector<int> left, right;
        for (int r : reqs) {
            if (r < headPos) left.push_back(r);
            else right.push_back(r);
        }

        if (algo == 3) {
            // SCAN
            if (direction == 1) {
                // 先向号增大方向：访问 right 按升序
                for (int r : right) order.push_back(r);
                // 然后向号减小方向：访问 left 按降序
                for (int i = (int)left.size() - 1; i >= 0; i--) {
                    order.push_back(left[i]);
                }
            } else {
                // direction == 0，先向号减小方向：访问 left 按降序
                for (int i = (int)left.size() - 1; i >= 0; i--) {
                    order.push_back(left[i]);
                }
                // 再向号增大方向：访问 right 按升序
                for (int r : right) order.push_back(r);
            }
        } else {
            // C-SCAN
            if (direction == 1) {
                // 先向号增大：访问 right 升序
                for (int r : right) order.push_back(r);
                // 到达最大后，跳到最小（wrap）：访问 left 升序
                for (int r : left) order.push_back(r);
            } else {
                // direction == 0，先向号减小：访问 left 降序
                for (int i = (int)left.size() - 1; i >= 0; i--) {
                    order.push_back(left[i]);
                }
                // 到达最小后，跳到最大（wrap）：访问 right 降序
                for (int i = (int)right.size() - 1; i >= 0; i--) {
                    order.push_back(right[i]);
                }
            }
        }
    } else {
        // 无效选项
        return 0;
    }

    // 计算总移动距离
    long long totalDistance = 0;
    for (int i = 1; i < (int)order.size(); i++) {
        totalDistance += llabs(order[i] - order[i - 1]);
    }

    // 输出访问路径
    for (int i = 0; i < (int)order.size(); i++) {
        cout << order[i];
        if (i + 1 < (int)order.size()) cout << ",";
    }
    cout << "\n";

    // 输出总移动距离
    cout << totalDistance << "\n";

    return 0;
}
