// Regression test for GitHub issue #14734: the extern type map used to be cached
// by value inside TypeLayoutContext, and the context is copied (with(),
// withSpecializationArgs(), ...) on the way to each lookup, so the map was rebuilt
// for every extern type encountered during layout. This program lays out several
// parameters of extern struct types and verifies that each one resolves to the
// linked-in definition from the separately compiled module: the element sizes of
// the buffers (two ints per element) and the field reads only come out right if
// every extern type was resolved.

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
//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4):name=fooBuf0
StructuredBuffer<Foo> fooBuf0;

//TEST_INPUT:ubuffer(data=[8 7 6 5 4 3 2 1], stride=4):name=fooBuf1
StructuredBuffer<Foo> fooBuf1;

//TEST_INPUT:ubuffer(data=[100 100 200 200 300 300 400 400], stride=4):name=barBuf
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
