//TEST:LANG_SERVER(filecheck=CHECK):
#language 202c

interface IWidth<let N : int>
{
}

__generic<T, let N : int>
extension vector<T, N> : IWidth<N>
{
}

struct Unavailable
{
}

struct Converted
{
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

void choose(Unavailable value)
{
}

void test()
{
//SIGNATURE:49,23
    choose(float3(0.0));
}

// In 2026 mode the legacy fallback makes the float3-to-Converted conversion viable and selects
// the second signature. In 202c both candidates are unmatched, so signature help retains the first
// declaration. This expectation therefore fails if signature help ignores the document version.
// CHECK: activeSignature: 0
// CHECK: (selected) func choose(Unavailable value) -> void
