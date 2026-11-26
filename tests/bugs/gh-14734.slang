// Tests end-to-end extern type resolution through slang-lib linking, added for
// the rework of the extern type map in issue #14734. The map's storage moved
// from TypeLayoutContext (a by-value-copied struct, so the lazy build was
// discarded on every copy) onto ProgramLayout, and its build now skips
// mangling declarations that cannot match. The performance aspect of #14734
// (layout time scaling with #extern lookups x #declarations) cannot be pinned
// by an output-comparison test; what this file locks is that several
// extern-typed buffer parameters still resolve to their linked-in definitions.

//TEST:COMPILE: -no-codegen tests/bugs/gh-14734-impl.slang -o tests/bugs/gh-14734-impl.slang-lib
//TEST:COMPARE_COMPUTE(targets=cpu, filecheck-buffer=CHECK):-xslang -r -xslang tests/bugs/gh-14734-impl.slang-lib -output-using-type

// The definition of Foo comes from the module compiled above.
extern struct Foo {};

// Accessors for Foo's fields, implemented in the linked-in module so that this
// module does not need to know them.
extern int getFooV(Foo f);
extern int getFooW(Foo f);

// A second extern type with a different linked-in definition.
extern struct Bar {};
extern int getBarSum(Bar b);

// Two ints per Foo element and per Bar element: the reads below only line up
// with this data if the linked-in definitions drove the element layouts.
//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=8):name=fooBuf0
StructuredBuffer<Foo> fooBuf0;

//TEST_INPUT:ubuffer(data=[8 7 6 5 4 3 2 1], stride=8):name=fooBuf1
StructuredBuffer<Foo> fooBuf1;

//TEST_INPUT:ubuffer(data=[100 100 200 200 300 300 400 400], stride=8):name=barBuf
StructuredBuffer<Bar> barBuf;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = int(dispatchThreadID.x);
    outputBuffer[i] = getFooV(fooBuf0[i]) + getFooW(fooBuf0[i]) +
                      getFooV(fooBuf1[i]) + getFooW(fooBuf1[i]) +
                      getBarSum(barBuf[i]);
}

// CHECK: type: int32_t
// CHECK: 218
// CHECK: 418
// CHECK: 618
// CHECK: 818
