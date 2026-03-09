#include <bits/stdc++.h>
using namespace std;

// Helper to split a comma-separated string into integers
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

    int algo;                // 1 = OPT, 2 = FIFO, 3 = LRU
    int frameCount;          // number of frames
    string seqLine;          // comma-separated page sequence

    // Read inputs
    if (!(cin >> algo)) return 0;
    cin >> frameCount;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, seqLine);

    vector<int> pages = parse_sequence(seqLine);
    int n = pages.size();

    vector<string> outputEntries;
    int pageFaults = 0;

    if (algo == 1) {
        // OPTIMAL replacement
        vector<int> frames(frameCount, -1);
        vector<int> insertTime(frameCount, -1);
        int currentTime = 0;

        for (int i = 0; i < n; i++) {
            int p = pages[i];
            bool hit = false;
            for (int f = 0; f < frameCount; f++) {
                if (frames[f] == p) {
                    hit = true;
                    break;
                }
            }
            if (!hit) {
                pageFaults++;
                int emptyIdx = -1;
                for (int f = 0; f < frameCount; f++) {
                    if (frames[f] == -1) {
                        emptyIdx = f;
                        break;
                    }
                }
                if (emptyIdx != -1) {
                    frames[emptyIdx] = p;
                    insertTime[emptyIdx] = currentTime++;
                } else {
                    int victim = 0, farthestNextUse = -1;
                    for (int f = 0; f < frameCount; f++) {
                        int nextUse = n + 1;
                        for (int k = i + 1; k < n; k++) {
                            if (pages[k] == frames[f]) {
                                nextUse = k;
                                break;
                            }
                        }
                        if (nextUse > farthestNextUse) {
                            farthestNextUse = nextUse;
                            victim = f;
                        } else if (nextUse == farthestNextUse) {
                            if (insertTime[f] < insertTime[victim]) {
                                victim = f;
                            }
                        }
                    }
                    frames[victim] = p;
                    insertTime[victim] = currentTime++;
                }
            }
            string entry;
            for (int f = 0; f < frameCount; f++) {
                if (frames[f] == -1) {
                    entry.push_back('-');
                } else {
                    entry += to_string(frames[f]);
                }
                if (f != frameCount - 1) entry.push_back(',');
            }
            entry.push_back(',');
            entry.push_back(hit ? '1' : '0');
            outputEntries.push_back(entry);
        }

    } else if (algo == 2) {
        // FIFO replacement
        vector<int> frames(frameCount, -1);
        queue<int> fifoQueue;
        int currentTime = 0;

        for (int i = 0; i < n; i++) {
            int p = pages[i];
            bool hit = false;
            for (int f = 0; f < frameCount; f++) {
                if (frames[f] == p) {
                    hit = true;
                    break;
                }
            }
            if (!hit) {
                pageFaults++;
                int emptyIdx = -1;
                for (int f = 0; f < frameCount; f++) {
                    if (frames[f] == -1) {
                        emptyIdx = f;
                        break;
                    }
                }
                if (emptyIdx != -1) {
                    frames[emptyIdx] = p;
                    fifoQueue.push(emptyIdx);
                } else {
                    int victim = fifoQueue.front();
                    fifoQueue.pop();
                    frames[victim] = p;
                    fifoQueue.push(victim);
                }
            }
            string entry;
            for (int f = 0; f < frameCount; f++) {
                if (frames[f] == -1) {
                    entry.push_back('-');
                } else {
                    entry += to_string(frames[f]);
                }
                if (f != frameCount - 1) entry.push_back(',');
            }
            entry.push_back(',');
            entry.push_back(hit ? '1' : '0');
            outputEntries.push_back(entry);
        }

    } else if (algo == 3) {
        // LRU replacement using an ordered list/vector
        vector<int> framesList;  // will hold up to frameCount pages, in order from LRU (front) to MRU (back)

        for (int i = 0; i < n; i++) {
            int p = pages[i];
            bool hit = false;
            // Check if p is already in framesList
            for (auto it = framesList.begin(); it != framesList.end(); ++it) {
                if (*it == p) {
                    hit = true;
                    // Move this page to the back (most recently used)
                    framesList.erase(it);
                    framesList.push_back(p);
                    break;
                }
            }
            if (!hit) {
                pageFaults++;
                if ((int)framesList.size() < frameCount) {
                    // Still space: just append
                    framesList.push_back(p);
                } else {
                    // Evict the front (least recently used)
                    framesList.erase(framesList.begin());
                    framesList.push_back(p);
                }
            }
            // Build output entry: print contents of framesList in order, then '-' for empty slots
            string entry;
            for (int f = 0; f < frameCount; f++) {
                if (f < (int)framesList.size()) {
                    entry += to_string(framesList[f]);
                } else {
                    entry.push_back('-');
                }
                if (f != frameCount - 1) entry.push_back(',');
            }
            entry.push_back(',');
            entry.push_back(hit ? '1' : '0');
            outputEntries.push_back(entry);
        }

    } else {
        // Invalid algorithm choice
        return 0;
    }

    // Print the slash-separated sequence
    for (size_t i = 0; i < outputEntries.size(); i++) {
        cout << outputEntries[i];
        if (i + 1 < outputEntries.size()) {
            cout << '/';
        }
    }
    cout << "\n";

    // Print total page faults
    cout << pageFaults << "\n";

    return 0;
}
