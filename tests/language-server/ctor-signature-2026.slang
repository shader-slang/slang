//TEST:LANG_SERVER(filecheck=CHECK):
#language 2026

interface IWidth<let N : int>
{
}

__generic<T, let N : int>
extension vector<T, N> : IWidth<N>
{
}

struct Fallback
{
}

struct Converted
{
}

extension Fallback
{
    __implicit_conversion(200)
    __init(float3 value)
    {
    }
}

extension Converted
{
    __implicit_conversion(100)
    __init<T : IFloat>(T value)
    {
    }
}

extension Converted
{
    __implicit_conversion(100)
    __init<T : IFloat, let N : int>(T value)
        where T : IWidth<N>
    {
    }
}

void choose(Converted value)
{
}

void choose(Fallback value)
{
}

void test()
{
//SIGNATURE:57,23
    choose(float3(0.0));
}

// Both conversions are viable in 2026. The lower-cost Converted conversion must win, providing
// the positive counterpart to the 202c document's Fallback selection.
// CHECK: (selected) func choose(Converted value) -> void
