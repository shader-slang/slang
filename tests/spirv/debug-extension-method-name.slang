//TEST:SIMPLE(filecheck=CHECK): -target spirv -g3 -O0 -emit-spirv-directly

// Regression test for #11563: SPIR-V debug function names for methods declared
// in an `extension` must be qualified with the extended type (e.g.
// "Example.extensionMethod"), matching methods declared in the struct body.
//
// The qualified-name checks are quote-anchored ("Example.<name>"). They are
// self-match-proof against the embedded source even at -g3: SPIR-V text escapes
// interior quotes (\") inside any embedded-source OpString, and the qualified
// string "Example.<name>" never appears verbatim in the source body (call sites
// use lowercase `example.`). Do NOT add a CHECK-NOT on the bare name — the
// embedded source would self-match it.

interface IBase
{
    int baseInterfaceMethod(int value);
}

struct Example : IBase
{
    int baseMethod(int value)
    {
        return value;
    }

    int baseInterfaceMethod(int value)
    {
        return value;
    }
}

extension Example
{
    int extensionMethod(int value)
    {
        return value;
    }
}

interface IExtension
{
    int extensionInterfaceMethod(int value);
}

extension Example : IExtension
{
    int extensionInterfaceMethod(int value)
    {
        return value;
    }
}

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Example example;
    outputBuffer[0] =
        example.baseMethod(1) +
        example.baseInterfaceMethod(2) +
        example.extensionMethod(3) +
        example.extensionInterfaceMethod(4);
}

// Struct-body methods are already qualified (these pass before and after the fix).
//CHECK-DAG: OpString "Example.baseMethod"
//CHECK-DAG: OpString "Example.baseInterfaceMethod"

// Extension methods must now be qualified too (the regression this test guards).
//CHECK-DAG: OpString "Example.extensionMethod"
//CHECK-DAG: OpString "Example.extensionInterfaceMethod"
//CHECK-DAG: OpName %{{.*}} "Example.extensionMethod"
//CHECK-DAG: OpName %{{.*}} "Example.extensionInterfaceMethod"
