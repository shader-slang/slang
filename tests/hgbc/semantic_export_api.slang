public struct ImportedCounter
{
    public uint value;
    [host] public __init(uint initial) { value = initial; }
    [host] [mutating] public void increment(uint amount) { value += amount; }
}
