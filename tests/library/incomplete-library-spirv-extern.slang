// An entry point that calls a body-less `extern` function compiles to SPIR-V under
// `-incomplete-library` at every `-O` level, and the function becomes an import declaration that
// a later `spirv-link` resolves. `mainLoop` and `mainNoLoop` reach two different SPIRV-Tools
// passes that cannot process a function declaration (dead-branch elimination at `-O1` and up,
// local single-store elimination at `-O2` and up).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainLoop -stage compute -incomplete-library -O0
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainLoop -stage compute -incomplete-library -O1
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainLoop -stage compute -incomplete-library -O2
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainLoop -stage compute -incomplete-library -O3
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainNoLoop -stage compute -incomplete-library -O0
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainNoLoop -stage compute -incomplete-library -O1
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainNoLoop -stage compute -incomplete-library -O2
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry mainNoLoop -stage compute -incomplete-library -O3
//TEST:COMPILE: tests/library/incomplete-library-spirv-extern.slang -target spirv -entry mainLoop -stage compute -incomplete-library -O1 -o tests/library/incomplete-library-spirv-extern-loop.spv
//TEST:COMPILE: tests/library/incomplete-library-spirv-extern.slang -target spirv -entry mainNoLoop -stage compute -incomplete-library -O2 -o tests/library/incomplete-library-spirv-extern-no-loop.spv

// A module that imports nothing is still optimized under `-incomplete-library`: the optimizer
// inlines `triple` into the entry point.
//TEST:SIMPLE(filecheck=OPT): -target spirv-asm -entry mainNoExtern -stage compute -incomplete-library -O2
// OPT-NOT: Linkage
// OPT: OpEntryPoint
// OPT-NOT: Linkage
// OPT-NOT: OpFunctionCall

// Without `-incomplete-library`, the unresolved extern is still an error.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=NOLIB): -target spirv -entry mainLoop -stage compute
// NOLIB: error[E45001]: unresolved external symbol

// CHECK: OpCapability Linkage
// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} LinkageAttributes "_S1fp1pi_uu" Import
// CHECK: OpFunction
// CHECK-NEXT: OpFunctionParameter
// CHECK-NEXT: OpFunctionEnd

[[vk::binding(0, 0)]] RWStructuredBuffer<uint> data;

extern uint f(uint x);

[shader("compute")]
[numthreads(1, 1, 1)]
void mainLoop(uint3 id: SV_DispatchThreadID)
{
    uint sum = 0u;
    for (uint i = 0u; i < 8u; i++)
        sum += f(id.x + i);
    data[id.x] = sum;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainNoLoop(uint3 id: SV_DispatchThreadID)
{
    data[id.x] = f(id.x);
}

uint triple(uint x)
{
    return x * 3u;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainNoExtern(uint3 id: SV_DispatchThreadID)
{
    uint sum = 0u;
    for (uint i = 0u; i < 8u; i++)
        sum += triple(id.x + i);
    data[id.x] = sum;
}
