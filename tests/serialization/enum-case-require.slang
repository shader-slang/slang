//TEST:COMPILE: tests/serialization/enum-case-require-module.slang -module-name enum_case_require_cached -o tests/serialization/enum_case_require_cached.slang-module
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry namedMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry aliasMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry castMain -stage compute -emit-spirv-directly -warnings-as-errors all
//DIAGNOSTIC_TEST:SIMPLE(diag=NAMED,non-exhaustive):-target spirv -entry namedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS,non-exhaustive):-target spirv -entry aliasMain -stage compute
// non-exhaustive: capability errors also include declaration/use notes.

// There is no source file with this module name, so these tests require deserialization.
import enum_case_require_cached;

RWStructuredBuffer<int> outputBuffer;

// HLSL: RWStructuredBuffer
[numthreads(1, 1, 1)]
void namedMain()
//NAMED: entrypoint 'namedMain' uses features that are not available
{
    outputBuffer[0] = readValue<Mode.A>();
}

[numthreads(1, 1, 1)]
void aliasMain()
//ALIAS: entrypoint 'aliasMain' uses features that are not available
{
    HlslAlias value = { 7 };
    outputBuffer[0] = value.payload;
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void castMain()
{
    outputBuffer[0] = fromTag<0>() + fromTag<42>();
    outputBuffer[1] = readValue<Mode.B>();
}
