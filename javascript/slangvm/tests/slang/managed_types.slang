public class Counter
{
    public int value;
    public __init(int initialValue) { value = initialValue; }
}

public struct CounterPair
{
    public Counter first;
    public Counter second;
}

public struct State
{
    public CounterPair pair;
    public RuntimeArray<Counter> counters;
    public String label;
}
