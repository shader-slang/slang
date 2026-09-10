//TEST:LANG_SERVER(filecheck=CHECK):
#language 202c

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

// In 2026 mode the legacy fallback makes the float3-to-Converted conversion viable and selects
// that lower-cost signature. In 202c that conversion is ambiguous, while the higher-cost Fallback
// conversion remains viable and is selected. This expectation therefore fails if signature help
// ignores the document version.
// CHECK: (selected) func choose(Fallback value) -> void
