//TEST:COMPILE: tests/bugs/13306/link-time-types.hlsl -Gec -no-codegen
//TEST:COMPILE: tests/bugs/13306/link-time-types.hlsl -Gec -target hlsl -entry main -validate-ir -DRESOLVE_DEFAULT_ALIAS
//TEST:COMPILE: tests/bugs/13306/link-time-types.hlsl -Gec -target spirv -entry main -validate-ir -DRESOLVE_DEFAULT_ALIAS

// The linker can replace `Alias` or `External` with another definition. Semantic checking
// permits copies of these types for both ordinary `static` variables and parameter shadows;
// an unknown final layout alone does not make mutable storage invalid.
interface IData
{
    uint getValue();
};
struct Data : IData
{
    uint value;
    uint getValue() { return value; }
};
extern struct Alias : IData = Data;
uniform Alias aliasInput;
static Alias mutableAlias;

#ifndef RESOLVE_DEFAULT_ALIAS
extern struct External {};
uniform External externalInput;
static External mutableExternal;

void copyExternal()
{
    externalInput = externalInput;
    mutableExternal = externalInput;
}
#endif

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    aliasInput = aliasInput;
    mutableAlias = aliasInput;
    output[tid.x] = aliasInput.getValue() + mutableAlias.getValue();
}
