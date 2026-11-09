//TEST:EXECUTABLE:

struct Pair { int first; int second; }

class Counter
{
    Pair pair;
    __init() { pair = Pair(0, 0); }
    void set(int value) { pair.first = value; this.pair.second = value + 1; }
    property value : int
    {
        get { return pair.first; }
        set { pair.first = newValue; }
    }
}

export __extern_cpp int main()
{
    let counter = new Counter();
    let alias = counter;
    counter.set(7);
    if (alias.pair.first != 7 || alias.pair.second != 8)
        return 1;
    counter.value = 11;
    if (alias.value != 11)
        return 2;
    counter.pair.second = 13;
    return alias.pair.second == 13 ? 0 : 3;
}
