// https://github.com/shader-slang/slang/issues/12764
//
// HLSL/C++ declare an indexing operator with `operator[]`. Slang spells this as
// a `__subscript` declaration. Porting such code should get one targeted
// diagnostic pointing at `__subscript`, not the bare "invalid operator '['"
// error followed by an "unexpected ']'" parse-error cascade.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

//CHECK: E20021
//CHECK: __subscript
struct Pair
{
    int2 values;

    int operator[](int index)
    {
        return values[index];
    }
}

// The old bare invalid-operator error and the follow-on unexpected-token parse
// error must not appear.
//CHECK-NOT: E20008
//CHECK-NOT: E20001
