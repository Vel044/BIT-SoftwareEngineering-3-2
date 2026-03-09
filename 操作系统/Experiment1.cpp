#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <climits>
using namespace std;

struct Process
{
    int pid;
    int arrival;
    int burst;    // 总运行时间
    int priority; // 初始优先级（或动态优先级初值）
    int ts;       // 时间片（调度算法4和5使用）

    // 以下用于模拟过程中使用
    int remaining; // 剩余执行时间
    int start;     // 当前段开始时间（用于合并段）
};

struct Segment
{
    int order;
    int pid;
    int start;
    int end;
    int printPriority; // 调度时输出的优先级
};

// -------------------- FCFS --------------------
// 先来先服务：按到达时间排序后，若当前时间小于进程到达时间则快进时间，调度后输出原始优先级
vector<Segment> fcfs(vector<Process> processes)
{
    // 按到达时间、pid排序
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         { return a.arrival == b.arrival ? a.pid < b.pid : a.arrival < b.arrival; });
    vector<Segment> segments;
    int time = 0, order = 1;
    for (auto &p : processes)
    {
        if (time < p.arrival)
            time = p.arrival;
        int endtime = time + p.burst;
        segments.push_back({order++, p.pid, time, endtime, p.priority});
        time = endtime;
    }
    return segments;
}

// -------------------- SJF --------------------
// 非抢占式短作业优先：在所有已到达且未完成的进程中选择 cost（burst）最小的
vector<Segment> sjf(vector<Process> processes)
{
    int n = processes.size();
    vector<bool> done(n, false);
    int completed = 0, order = 1;
    vector<Segment> segments;
    // 为方便，记录原始索引顺序（排序后下标发生变化）
    vector<Process> procs = processes;
    // 按到达时间、burst、pid排序
    sort(procs.begin(), procs.end(), [](const Process &a, const Process &b)
         {
        if(a.arrival == b.arrival) {
            if(a.burst == b.burst)
                return a.pid < b.pid;
            return a.burst < b.burst;
        }
        return a.arrival < b.arrival; });
    int time = procs[0].arrival;
    while (completed < n)
    {
        // 找出所有已到达且未完成的进程
        vector<int> avail;
        for (int i = 0; i < n; i++)
        {
            if (!done[i] && procs[i].arrival <= time)
                avail.push_back(i);
        }
        if (avail.empty())
        {
            // 没有进程到达，快进到下一个未完成进程的到达时间
            for (int i = 0; i < n; i++)
            {
                if (!done[i])
                {
                    time = procs[i].arrival;
                    avail.push_back(i);
                    break;
                }
            }
        }
        // 从 avail 中选出 burst 最小的（若相等则 pid 小者）
        int mini = avail[0];
        for (int idx : avail)
        {
            if (procs[idx].burst < procs[mini].burst ||
                (procs[idx].burst == procs[mini].burst && procs[idx].pid < procs[mini].pid))
                mini = idx;
        }
        int start = time;
        int endtime = time + procs[mini].burst;
        segments.push_back({order++, procs[mini].pid, start, endtime, procs[mini].priority});
        time = endtime;
        done[mini] = true;
        completed++;
    }
    return segments;
}

// -------------------- SRT --------------------
// 抢占式短作业优先：按单位时间模拟，选出剩余时间最短的运行，若有切换则记录上一个段
vector<Segment> srt(vector<Process> processes)
{
    int n = processes.size();
    vector<bool> done(n, false);
    // 为模拟方便，复制一份并初始化 remaining
    vector<Process> procs = processes;
    for (int i = 0; i < n; i++)
    {
        procs[i].remaining = procs[i].burst;
    }
    // 按到达时间、burst、pid排序
    sort(procs.begin(), procs.end(), [](const Process &a, const Process &b)
         {
        if(a.arrival == b.arrival) {
            if(a.burst == b.burst)
                return a.pid < b.pid;
            return a.burst < b.burst;
        }
        return a.arrival < b.arrival; });
    int time = procs[0].arrival;
    int completed = 0, order = 1;
    int current = -1; // 当前正在运行进程的下标
    int segStart = time;
    vector<Segment> segments;
    while (completed < n)
    {
        // 找出所有已到达且未完成的进程
        int mini = -1;
        int minRem = INT_MAX;
        for (int i = 0; i < n; i++)
        {
            if (!done[i] && procs[i].arrival <= time && procs[i].remaining < minRem)
            {
                minRem = procs[i].remaining;
                mini = i;
            }
        }
        if (mini == -1)
        {
            // 没有可运行进程，快进时间
            for (int i = 0; i < n; i++)
            {
                if (!done[i])
                {
                    time = procs[i].arrival;
                    break;
                }
            }
            continue;
        }
        // 若切换进程，则记录上一个进程的段
        if (current != mini)
        {
            if (current != -1 && segStart < time)
            {
                segments.push_back({order++, procs[current].pid, segStart, time, procs[current].priority});
            }
            current = mini;
            segStart = time;
        }
        // 决定运行时间：到下一个进程到达时刻或运行完当前进程
        int nextArrival = INT_MAX;
        for (int i = 0; i < n; i++)
        {
            if (!done[i] && procs[i].arrival > time && procs[i].arrival < nextArrival)
                nextArrival = procs[i].arrival;
        }
        int runTime = (nextArrival == INT_MAX) ? procs[mini].remaining : min(procs[mini].remaining, nextArrival - time);
        if (runTime <= 0)
            runTime = procs[mini].remaining;
        procs[mini].remaining -= runTime;
        time += runTime;
        if (procs[mini].remaining == 0)
        {
            segments.push_back({order++, procs[mini].pid, segStart, time, procs[mini].priority});
            done[mini] = true;
            completed++;
            current = -1;
        }
    }
    return segments;
}

// -------------------- RR --------------------
// 时间片轮转：按到达顺序进入队列，每次取队首进程运行一个时间片（或剩余时间更短）
vector<Segment> rr(vector<Process> processes)
{
    int n = processes.size();
    // 按到达时间排序
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         { return a.arrival < b.arrival; });
    // 初始化 remaining 为 burst
    for (auto &p : processes)
        p.remaining = p.burst;
    queue<Process *> q;
    int time = processes[0].arrival;
    int order = 1;
    int idx = 0; // 扫描 processes 的下标
    vector<Segment> segments;
    // 将所有到达时间 <= time 的进程入队
    while (idx < n && processes[idx].arrival <= time)
    {
        q.push(&processes[idx]);
        idx++;
    }
    while (!q.empty())
    {
        Process *cur = q.front();
        q.pop();
        if (time < cur->arrival)
            time = cur->arrival;
        if (cur->remaining > cur->ts)
        {
            segments.push_back({order++, cur->pid, time, time + cur->ts, cur->priority});
            time += cur->ts;
            cur->remaining -= cur->ts;
            // 将新到达的进程入队
            while (idx < n && processes[idx].arrival <= time)
            {
                q.push(&processes[idx]);
                idx++;
            }
            q.push(cur);
        }
        else
        {
            segments.push_back({order++, cur->pid, time, time + cur->remaining, cur->priority});
            time += cur->remaining;
            cur->remaining = 0;
            while (idx < n && processes[idx].arrival <= time)
            {
                q.push(&processes[idx]);
                idx++;
            }
        }
    }
    return segments;
}

// -------------------- DP --------------------
// 动态优先级调度：
// 1. 进程按到达时间排序；
// 2. 对于所有已到达进程（下标小于 i），若其到达时间在上次调度时间 last 与当前时间之间，则将其优先级减 1（但不低于 0）；
// 3. 在已到达且未完成进程中选择 priority 最小者；
// 4. 如果选中进程的剩余运行时间 <= 时间片，则调度完毕，否则运行一个时间片；
// 5. 调度后对选中进程优先级加 3，其它未完成进程优先级减 1（但不低于 0）。
vector<Segment> dp(vector<Process> processes)
{
    int n = processes.size();
    // 按到达时间、pid 升序排序
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         { return a.arrival == b.arrival ? a.pid < b.pid : a.arrival < b.arrival; });
    // 初始化 remaining 为 burst
    for (int i = 0; i < n; i++)
    {
        processes[i].remaining = processes[i].burst;
    }
    vector<bool> done(n, false);
    int time = processes[0].arrival;
    int last = time;
    int completed = 0;
    int order = 1;
    vector<Segment> segments;
    while (completed < n)
    {
        // 计算所有已到达进程个数 i，同时更新那些在 (last, time) 内到达的进程优先级减 1（若 > 1，否则为 0）
        int i = 0;
        for (; i < n && processes[i].arrival <= time; i++)
        {
            if (last < processes[i].arrival && processes[i].arrival < time)
            {
                if (processes[i].priority > 1)
                    processes[i].priority--;
                else
                    processes[i].priority = 0;
            }
        }
        // 在已到达的进程中选出未完成且 priority 最小的
        int mini = -1;
        int minPriority = INT_MAX;
        for (int j = 0; j < i; j++)
        {
            if (!done[j] && processes[j].priority < minPriority)
            {
                minPriority = processes[j].priority;
                mini = j;
            }
        }
        if (mini == -1)
        {
            // 没有可调度进程，跳到下一个进程到达时刻
            if (i < n)
                time = processes[i].arrival;
            continue;
        }
        last = time;
        int runTime = 0;
        // 若剩余时间 <= 时间片，则调度完，否则运行一个时间片
        if (processes[mini].remaining <= processes[mini].ts)
        {
            runTime = processes[mini].remaining;
            segments.push_back({order++, processes[mini].pid, time, time + runTime, processes[mini].priority + 3});
            time += runTime;
            processes[mini].remaining = 0;
            done[mini] = true;
            completed++;
        }
        else
        {
            runTime = processes[mini].ts;
            segments.push_back({order++, processes[mini].pid, time, time + runTime, processes[mini].priority + 3});
            time += runTime;
            processes[mini].remaining -= runTime;
        }
        // 调度后更新所有已到达且未完成的进程优先级：
        // 若为选中进程则加 3，否则减 1（但不低于 0）
        for (int j = 0; j < i; j++)
        {
            if (!done[j])
            {
                if (j == mini)
                    processes[j].priority += 3;
                else
                {
                    if (processes[j].priority > 1)
                        processes[j].priority--;
                    else
                        processes[j].priority = 0;
                }
            }
        }
    }
    return segments;
}

// -------------------- main --------------------
int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int dispatchType;
    cin >> dispatchType;
    cin.ignore(); // 忽略行末换行

    vector<Process> processes;
    string line;
    while (getline(cin, line))
    {
        if (line.empty())
            continue;
        // 输入格式：pid/arrival/burst/priority/ts
        Process p;
        char slash;
        stringstream ss(line);
        ss >> p.pid >> slash >> p.arrival >> slash >> p.burst >> slash >> p.priority >> slash >> p.ts;
        processes.push_back(p);
    }

    vector<Segment> segments;
    switch (dispatchType)
    {
    case 1:
        segments = fcfs(processes);
        break;
    case 2:
        segments = sjf(processes);
        break;
    case 3:
        segments = srt(processes);
        break;
    case 4:
        segments = rr(processes);
        break;
    case 5:
        segments = dp(processes);
        break;
    default:
        break;
    }

    // 输出所有调度段，每段格式：order/pid/start/end/printPriority
    for (auto &seg : segments)
    {
        cout << seg.order << "/" << seg.pid << "/" << seg.start << "/" << seg.end << "/" << seg.printPriority << "\n";
    }

    return 0;
}
