import managed_types;

[host]
State makeState(int value)
{
    Counter counter = new Counter(value);
    RuntimeArray<Counter> counters = RuntimeArray<Counter>::alloc(2);
    counters[0] = counter;
    counters[1] = counter;
    String pattern = "state {}";
    String label = hgbc_asm { result : $$String = OpFormatString $pattern $$int $value; };
    return State(CounterPair(counter, counter), counters, label);
}

[host] Counter getCounter(State state) { return state.pair.first; }
[host] RuntimeArray<Counter> getCounters(State state) { return state.counters; }
[host] String getLabel(State state) { return state.label; }
[host] String makeLiteral() { return "literal text"; }

[host]
State makeThenFail(int value, int divisor)
{
    State state = makeState(value);
    state.pair.first.value = 24 / divisor;
    return state;
}
