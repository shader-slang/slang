// https://github.com/shader-slang/slang/issues/12764
//
// Slang has order-independent lookup, so a C/HLSL-style struct forward
// declaration is redundant. Porting HLSL that forward-declares a struct and
// then defines it should produce ONE clear migration diagnostic pointing at the
// redundant forward declaration, and must NOT additionally report the later use
// of the type as an ambiguous reference (the old E39999 cascade).

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

//CHECK: E30203
//CHECK: Slang does not require or support this struct forward declaration
struct Item;

struct Item
{
    uint value;
}

Item makeItem(uint value)
{
    Item result;
    result.value = value;
    return result;
}

// The reference to `Item` must not be diagnosed as ambiguous once the redundant
// forward declaration has been reported as the single root cause.
//CHECK-NOT: E39999
//CHECK-NOT: ambiguous reference
