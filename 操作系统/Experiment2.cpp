#include <iostream>
#include <sstream>
#include <list>
#include <climits>

using namespace std;

struct MemoryBlock
{
    int start;
    int end;
    bool is_free;
    int process_id;

    MemoryBlock(int s, int e, bool free, int pid)
        : start(s), end(e), is_free(free), process_id(pid) {}
};

list<MemoryBlock> memory;

void allocate(list<MemoryBlock>::iterator it, int size, int pid)
{
    int original_start = it->start;
    int original_end = it->end;
    int allocated_end = original_start + size - 1;

    MemoryBlock allocated(original_start, allocated_end, false, pid);
    it = memory.erase(it);
    memory.insert(it, allocated);

    if (allocated_end + 1 <= original_end)
    {
        memory.insert(it, MemoryBlock(allocated_end + 1, original_end, true, -1));
    }
}

void merge_blocks(list<MemoryBlock>::iterator it)
{
    bool merged;
    do
    {
        merged = false;
        if (it != memory.begin())
        {
            auto prev_it = prev(it);
            if (prev_it->is_free && prev_it->end + 1 == it->start)
            {
                prev_it->end = it->end;
                memory.erase(it);
                it = prev_it;
                merged = true;
            }
        }

        auto next_it = next(it);
        if (next_it != memory.end() && it->is_free && next_it->is_free &&
            it->end + 1 == next_it->start)
        {
            it->end = next_it->end;
            memory.erase(next_it);
            merged = true;
        }
    } while (merged);
}

string get_memory_state()
{
    stringstream ss;
    bool first = true;
    for (const auto &block : memory)
    {
        if (!first)
            ss << "/";
        first = false;
        ss << block.start << "-" << block.end << ".";
        if (block.is_free)
            ss << "0";
        else
            ss << "1." << block.process_id;
    }
    return ss.str();
}

int main()
{
    int algorithm, initial_memory;
    cin >> algorithm >> initial_memory;
    memory.emplace_back(0, initial_memory - 1, true, -1);

    string line;
    cin.ignore();
    while (getline(cin, line))
    {
        if (line.empty())
            continue;

        int seq, pid, op, size;
        sscanf(line.c_str(), "%d/%d/%d/%d", &seq, &pid, &op, &size);

        if (op == 1)
        { // Allocation
            bool success = false;
            list<MemoryBlock>::iterator target_it = memory.end();

            if (algorithm == 1)
            { // First-fit
                for (auto it = memory.begin(); it != memory.end(); ++it)
                {
                    if (it->is_free && (it->end - it->start + 1) >= size)
                    {
                        target_it = it;
                        success = true;
                        break;
                    }
                }
            }
            else if (algorithm == 2)
            { // Best-fit
                int min_size = INT_MAX;
                for (auto it = memory.begin(); it != memory.end(); ++it)
                {
                    if (it->is_free)
                    {
                        int block_size = it->end - it->start + 1;
                        if (block_size >= size && block_size < min_size)
                        {
                            min_size = block_size;
                            target_it = it;
                            success = true;
                        }
                    }
                }
            }
            else
            { // Worst-fit
                int max_size = -1;
                for (auto it = memory.begin(); it != memory.end(); ++it)
                {
                    if (it->is_free)
                    {
                        int block_size = it->end - it->start + 1;
                        if (block_size >= size && block_size > max_size)
                        {
                            max_size = block_size;
                            target_it = it;
                            success = true;
                        }
                    }
                }
            }

            if (success)
                allocate(target_it, size, pid);
        }
        else if (op == 2)
        { // Deallocation
            for (auto it = memory.begin(); it != memory.end(); ++it)
            {
                if (!it->is_free && it->process_id == pid &&
                    (it->end - it->start + 1) == size)
                {
                    it->is_free = true;
                    merge_blocks(it);
                    break;
                }
            }
        }

        cout << seq << "/" << get_memory_state() << endl;
    }
}