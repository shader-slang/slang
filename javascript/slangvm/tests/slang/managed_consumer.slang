import managed_types;

[host] State copyState(State state) { return state; }
[host] void advance(State state, int amount) { state.pair.first.value += amount; }
[host] int readState(State state)
{
    return state.pair.first.value + state.pair.second.value +
        state.counters[0].value + state.counters[1].value;
}
[host] int readCounter(Counter counter) { return counter.value; }
[host] void advanceCounter(Counter counter, int amount) { counter.value += amount; }
[host] int readArray(RuntimeArray<Counter> counters)
{
    return counters[0].value + counters[1].value;
}
[host] void advanceArray(RuntimeArray<Counter> counters, int amount)
{
    counters[1].value += amount;
}
[host] uint textBytes(String text)
{
    return hgbc_asm { result : $$uint = OpStringGetByteCount $text; };
}
[host] String copyText(String text) { return text; }
