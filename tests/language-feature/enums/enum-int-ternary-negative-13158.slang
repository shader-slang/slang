// Negative companion for shader-slang/slang#13158: common-type inference for
// `?:`/`select` proposes a scalar common type but must not bypass coercion
// legality. An int-tagged enum cannot convert to the `uint` the arms join to
// (only enum->its-int-tag is allowed), so the scalar overload is inapplicable
// and a clean per-arm error names the enum type.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

enum E : int
{
    A = 1,
    B = 2,
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    bool cond = true;
    output[0] = cond ? E::A : 0u;
    output[1] = select(cond, E::B, 0u);
}

//CHECK-NOT: 30056
//CHECK-NOT: vector<
//CHECK: E30019
//CHECK: expected an expression of type 'uint', got 'E'
//CHECK-NOT: 30056
//CHECK-NOT: vector<
//CHECK: E30019
//CHECK: expected an expression of type 'uint', got 'E'
//CHECK-NOT: 30056
//CHECK-NOT: vector<
