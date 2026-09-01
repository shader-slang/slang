//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression for shader-slang/slang#11844: a struct that mixes bitfields with a
// trailing normal field must NOT emit warning E41021 ("default initializer will
// not initialize field '$bit_field_backing_0'") for `{}`. The synthesized
// constructor now zero-initializes the bitfield backing word, so the field is
// fully initialized. With exhaustive diagnostic matching (no annotations below),
// this test fails if any diagnostic — including E41021 — is emitted.

struct Test
{
    uint a : 10;
    uint b : 22;
    uint c;
};

void test()
{
    Test t = {};
}
