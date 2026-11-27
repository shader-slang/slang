//TEST:COMPILE: tests/serialization/enum-case-require-module.slang -module-name enum_case_require_cached -o tests/serialization/enum_case_require_cached.slang-module
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry namedMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry aliasMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultsMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry castMain -stage compute -emit-spirv-directly -warnings-as-errors all
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry overridesMain -stage compute -emit-spirv-directly -warnings-as-errors all
//DIAGNOSTIC_TEST:SIMPLE(diag=NAMED,non-exhaustive):-target spirv -entry namedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS,non-exhaustive):-target spirv -entry aliasMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMETER_DEFAULT,non-exhaustive):-target spirv -entry parameterDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_DEFAULT,non-exhaustive):-target spirv -entry genericDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_DEFAULT,non-exhaustive):-target spirv -entry foldedDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE_DEFAULT,non-exhaustive):-target spirv -entry typeDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED_DEFAULT,non-exhaustive):-target spirv -entry nestedDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT_GENERIC_DEFAULT,non-exhaustive):-target spirv -entry explicitGenericDefaultMain -stage compute
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

[numthreads(1, 1, 1)]
void overridesMain()
{
    outputBuffer[0] = useDefault(Mode(1));
    outputBuffer[1] = readDefault<Mode(1)>();
    outputBuffer[2] = readFoldedDefault<0>();
    outputBuffer[3] = readTypeDefault<int>();
    outputBuffer[4] = useNestedDefault(0);
    outputBuffer[5] = useExplicitGenericDefault(0);
}

[numthreads(1, 1, 1)]
void parameterDefaultMain()
//PARAMETER_DEFAULT: entrypoint 'parameterDefaultMain' uses features that are not available
{
    outputBuffer[0] = useDefault();
}

[numthreads(1, 1, 1)]
void genericDefaultMain()
//GENERIC_DEFAULT: entrypoint 'genericDefaultMain' uses features that are not available
{
    outputBuffer[0] = readDefault();
}

[numthreads(1, 1, 1)]
void foldedDefaultMain()
//FOLDED_DEFAULT: entrypoint 'foldedDefaultMain' uses features that are not available
{
    outputBuffer[0] = readFoldedDefault();
}

[numthreads(1, 1, 1)]
void typeDefaultMain()
//TYPE_DEFAULT: entrypoint 'typeDefaultMain' uses features that are not available
{
    outputBuffer[0] = readTypeDefault();
}

[numthreads(1, 1, 1)]
void nestedDefaultMain()
//NESTED_DEFAULT: entrypoint 'nestedDefaultMain' uses features that are not available
{
    outputBuffer[0] = useNestedDefault();
}

[numthreads(1, 1, 1)]
void explicitGenericDefaultMain()
//EXPLICIT_GENERIC_DEFAULT: entrypoint 'explicitGenericDefaultMain' uses features that are not available
{
    outputBuffer[0] = useExplicitGenericDefault();
}

[numthreads(1, 1, 1)]
void defaultsMain()
{
    outputBuffer[0] = useDefault();
    outputBuffer[1] = readDefault();
    outputBuffer[2] = readFoldedDefault();
    outputBuffer[3] = readTypeDefault();
    outputBuffer[4] = useNestedDefault();
    outputBuffer[5] = useExplicitGenericDefault();
}
