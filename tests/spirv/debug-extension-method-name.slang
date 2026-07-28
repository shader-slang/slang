//TEST:SIMPLE(filecheck=CHECK): -target spirv -g3 -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute

// Regression test for shader-slang/slang#11563: name hints for methods declared
// in an `extension` must be qualified with the extended type (e.g.
// "Example.extensionMethod"), matching methods declared in the struct body.
//
// The name hint feeds both SPIR-V debug names (OpString / OpName / DebugFunction)
// and the C-like emitters' generated identifiers (HLSL/CUDA/etc.), so both the
// spirv-asm CHECK pass and the HLSL pass below guard the change.
//
// The qualified-name checks are quote-anchored ("Example.<name>"). They are
// self-match-proof against the -g3 embedded source: SPIR-V text escapes interior
// quotes (\") inside any embedded-source OpString, and the qualified string
// "Example.<name>" never appears verbatim in the source body (call sites use
// lowercase `example.`). Likewise "Box.boxExt" never appears verbatim (call site
// is `b.boxExt`). Do NOT add a CHECK-NOT on the bare name — the embedded source
// would self-match it.

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

// A generic struct + extension: the extension target is a `DeclRefType` of a
// `ContainerDecl`, so the method qualifies. The qualifier drops the generic
// arguments (`Box`, not `Box<int>`), matching how struct-body methods on
// generics and the symbol mangler name them.
struct Box<T>
{
    T value;
}

extension<T> Box<T>
{
    int boxExt(int value)
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
    Box<int> box;
    outputBuffer[0] =
        example.baseMethod(1) +
        example.baseInterfaceMethod(2) +
        example.extensionMethod(3) +
        example.extensionInterfaceMethod(4) +
        box.boxExt(5);
}

// --- SPIR-V debug names (-g3) ---
// All four Example methods are qualified the same way (OpString + OpName);
// the struct-body baselines pass before and after the fix, the extension
// methods are the regression this test guards.
//CHECK-DAG: OpString "Example.baseMethod"
//CHECK-DAG: OpString "Example.baseInterfaceMethod"
//CHECK-DAG: OpString "Example.extensionMethod"
//CHECK-DAG: OpString "Example.extensionInterfaceMethod"
//CHECK-DAG: OpName %{{.*}} "Example.baseMethod"
//CHECK-DAG: OpName %{{.*}} "Example.baseInterfaceMethod"
//CHECK-DAG: OpName %{{.*}} "Example.extensionMethod"
//CHECK-DAG: OpName %{{.*}} "Example.extensionInterfaceMethod"
// Extension on a generic type qualifies by the (un-specialized) type name.
//CHECK-DAG: OpString "Box.boxExt"

// --- C-like (HLSL) generated identifiers ---
// The same name hint qualifies the emitted identifier; `appendScrubbedName`
// converts the `.` to `_`, so the extension method is `Example_extensionMethod`,
// not bare `extensionMethod`. Guards the cross-target half of the change.
//HLSL-DAG: Example_baseMethod
//HLSL-DAG: Example_baseInterfaceMethod
//HLSL-DAG: Example_extensionMethod
//HLSL-DAG: Example_extensionInterfaceMethod
//HLSL-DAG: Box_boxExt
