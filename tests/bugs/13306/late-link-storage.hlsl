//TEST:COMPILE: tests/bugs/13306/late-link-storage-definition.slang -no-codegen -DUNSIZED_DEFINITION -o tests/bugs/13306/late-link-unsized.slang-lib
//TEST:COMPILE: tests/bugs/13306/late-link-storage-definition.slang -no-codegen -DPARAMETER_BLOCK_DEFINITION -o tests/bugs/13306/late-link-parameter-block.slang-lib
//TEST:COMPILE: tests/bugs/13306/late-link-storage-definition.slang -no-codegen -o tests/bugs/13306/late-link-sized.slang-lib
//TEST:SIMPLE_EX(filecheck=UNSIZED): tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-unsized.slang-lib -target spirv -entry main -validate-ir
//TEST:SIMPLE_EX(filecheck=UNSIZED): tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-unsized.slang-lib -target spirv -entry main -validate-ir -DINITIALIZED_STATIC
//TEST:SIMPLE_EX(filecheck=UNSIZED): tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-unsized.slang-lib -target spirv -entry main -validate-ir -Gec -DUSE_SHADOW
//TEST:SIMPLE_EX(filecheck=UNSIZED): tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-unsized.slang-lib -target cpp -entry main -validate-ir -Gec -DUSE_SHADOW -DDEFAULT_DATA
//TEST:SIMPLE_EX(filecheck=OPAQUE): tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-parameter-block.slang-lib -target spirv -entry main -validate-ir -DINITIALIZED_STATIC
//TEST:SIMPLE_EX(filecheck=OPAQUE): tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-parameter-block.slang-lib -target spirv -entry main -validate-ir -Gec -DUSE_SHADOW
//TEST:COMPILE: tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-sized.slang-lib -target cpp -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-sized.slang-lib -target cpp -entry main -validate-ir -DINITIALIZED_STATIC
//TEST:COMPILE: tests/bugs/13306/late-link-storage.hlsl -r tests/bugs/13306/late-link-sized.slang-lib -target cpp -entry main -validate-ir -Gec -DUSE_SHADOW -DDEFAULT_DATA

// An exporting module can define `Thing` with an inherited trailing array or a `ParameterBlock`.
// Semantic checking examines an incomplete declaration or a numeric default. Linked-type
// validation must check the actual storage required by both ordinary `static` variables
// and compatibility shadows. The same declarations remain valid when `Thing` contains a `uint`.
#ifdef DEFAULT_DATA
struct DefaultData
{
    uint value;
};
extern struct Thing = DefaultData;
#else
extern struct Thing {};
#endif
extern uint readThing(Thing* value);
uniform Thing input;
RWStructuredBuffer<uint> output;

#ifndef USE_SHADOW
#ifdef INITIALIZED_STATIC
static Thing temp = input;
#else
static Thing temp;
#endif
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
#ifdef USE_SHADOW
    input = input;
    output[0] = readThing(&input);
#else
#ifndef INITIALIZED_STATIC
    temp = input;
#endif
    output[0] = readThing(&temp);
#endif
}

// UNSIZED: error[E30074]
// UNSIZED-NOT: Slang compilation aborted
// OPAQUE: error[E30089]
// OPAQUE-NOT: Slang compilation aborted
