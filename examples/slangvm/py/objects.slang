class Counter
{
    int value;
    __init(int initial) { value = initial; }
}

struct SharedCounters
{
    Counter first;
    Counter second;
}

[host]
SharedCounters makeCounters(int initial)
{
    Counter counter = new Counter(initial);
    return SharedCounters(counter, counter);
}

[host]
Counter getFirst(SharedCounters counters) { return counters.first; }

[host]
void add(Counter counter, int amount) { counter.value += amount; }

[host]
int read(Counter counter) { return counter.value; }

[host]
int sum(SharedCounters counters) { return counters.first.value + counters.second.value; }
