//TEST:COMPILE: tests/bugs/13306/late-link-resource.slang -no-codegen -o tests/bugs/13306/late-link-resource.slang-lib
//TEST:COMPILE: tests/bugs/13306/late-link-data.slang -no-codegen -o tests/bugs/13306/late-link-data.slang-lib
//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/late-link-types.hlsl -r tests/bugs/13306/late-link-resource.slang-lib -target spirv -entry main -validate-ir
//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/late-link-types.hlsl -r tests/bugs/13306/late-link-resource.slang-lib -target spirv -entry main -validate-ir -DINITIALIZED_STATIC
//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/late-link-types.hlsl -r tests/bugs/13306/late-link-resource.slang-lib -target spirv -entry main -validate-ir -Gec -DUSE_SHADOW
//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/late-link-types.hlsl -r tests/bugs/13306/late-link-resource.slang-lib -target hlsl -entry main -validate-ir -Gec -DUSE_SHADOW -DDEFAULT_DATA
//TEST:COMPILE: tests/bugs/13306/late-link-types.hlsl -r tests/bugs/13306/late-link-data.slang-lib -target spirv -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/late-link-types.hlsl -r tests/bugs/13306/late-link-data.slang-lib -target spirv -entry main -validate-ir -DINITIALIZED_STATIC
//TEST:COMPILE: tests/bugs/13306/late-link-types.hlsl -r tests/bugs/13306/late-link-data.slang-lib -target spirv -entry main -validate-ir -Gec -DUSE_SHADOW -DDEFAULT_DATA

// The final definition of `Thing` comes from a separately compiled module. Semantic checking
// cannot classify its resource fields from this declaration or from the optional numeric
// default. Linked-type validation must reject unsupported mutable storage for ordinary
// `static` variables and compatibility shadows while allowing numeric definitions.
#ifdef DEFAULT_DATA
struct DefaultData
{
    uint value;
};
extern struct Thing = DefaultData;
#else
extern struct Thing {};
#endif
extern float4 readThing(Thing value);
uniform Thing input;
RWStructuredBuffer<float4> output;

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
    output[0] = readThing(input);
#else
#ifndef INITIALIZED_STATIC
    temp = input;
#endif
    output[0] = readThing(temp);
#endif
}

// CHECK: error[E30089]
// CHECK-NOT: Slang compilation aborted
