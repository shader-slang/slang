// Regression test for shader-slang/slang#12803 (and the residual half of #8125 / #12667 caveat 1).
//
// An empty struct occupies 0 bytes in Slang's type-layout rules, but a C++/CUDA empty-struct
// member still occupies 1 byte (sizeof(struct{}) == 1). Empty-struct legalization
// (IREmptyTypeLegalizationContext::isSimpleType in slang-ir-legalize-types.cpp) decides whether to
// drop an empty member or keep it as a 1-byte member. It used to KEEP the member whenever the
// struct carried a linkage/layout decoration in a fixed allowlist (Layout, Public, ExternCpp,
// Dll*, HLSLExport, BinaryInterfaceType). That made the retain-vs-drop choice depend on the
// spelling of the struct (`export`/`public`/plain) rather than on the fact that it is zero-storage.
//
// Two consequences: (1) a retained empty member shifts every following field 1 byte past its
// reflected offset (CUDA_ERROR_ILLEGAL_ADDRESS / SIGSEGV — #8125); (2) because the choice is
// decoration-driven and CUDA payload transfer is sized per separately-compiled stage, an empty
// payload field kept in one stage but dropped in another desyncs the register-vs-pointer transfer
// path across linked ray-tracing stages (#12803). #12304 fixed only the `public` spelling, and did
// so at the producer (it stopped emitting IRPublicDecoration). The `export`/[HLSLExport] spelling
// still reproduced the retention (#12667 caveat 1). The fix makes isSimpleType return false for any
// zero-field struct, so an empty struct legalizes away identically for every linkage spelling and
// every separately-compiled stage.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -shaderobj
// The reported failure is on CUDA (a distinct emitter); assert at the text-emit level that the
// empty members are gone from every emitted CUDA struct regardless of spelling (no GPU needed).
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -line-directive-mode none

// An empty struct spelled `export`: it carries [hlslExport], so isSimpleType kept it and the emitted
// CUDA struct retained the 1-byte member. This is the shape that still failed after #12304 (whose
// producer-side fix only covered `public`).
export struct EmptyExport
{
}

// An empty struct spelled `public`.
public struct EmptyPublic
{
}

// A plain empty struct with no linkage decoration.
struct EmptyPlain
{
}

// A struct whose only field is itself an empty struct — also zero-storage, must legalize away.
struct EmptyNested
{
    EmptyExport inner;
}

// Shape 1 (the reported crash shape, `export` spelling): an empty member preceding a buffer in a
// ParameterBlock element. If the empty member survives, `_result` lands 1 byte past its reflected
// offset and the readback reads out of bounds.
struct CallData
{
    EmptyExport p;
    RWStructuredBuffer<float> _result;
}

//TEST_INPUT: set call_data = new{ out ubuffer(data=[0 0], stride=4) }
ParameterBlock<CallData> call_data;

// Shape 2: an empty `export` member between two real fields — dropping it must keep `b` at the
// offset reflection reports so it still reads back.
struct Mid
{
    float a;
    EmptyExport e;
    float b;
}

// Shape 3: a nested empty (a struct whose only field is an empty struct) between two real fields.
struct MidNested
{
    float a;
    EmptyNested e;
    float b;
}

// Shape 4: a `public`-spelled empty member — already fixed by #12304, kept here as a companion so
// the CUDA-emit assertions cover every spelling in one place.
struct MidPublic
{
    float a;
    EmptyPublic e;
    float b;
}

// Results are read back through this top-level buffer.
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Shape 1: write through the buffer following the empty member; its offset must be 0.
    call_data._result[0] = 7.0;
    outputBuffer[0] = call_data._result[0];
    // CHECK: 7.0

    // Shape 2: `b` after an empty `export` member must read back.
    Mid m;
    m.a = 30.0;
    m.e = {};
    m.b = 40.0;
    outputBuffer[1] = m.b;
    // CHECK: 40.0

    // Shape 3: `b` after a nested-empty member must read back.
    MidNested mn;
    mn.a = 50.0;
    mn.e = {};
    mn.b = 60.0;
    outputBuffer[2] = mn.b;
    // CHECK: 60.0

    // Shape 4: `b` after a `public` empty member must read back.
    MidPublic mp;
    mp.a = 70.0;
    mp.e = {};
    mp.b = 80.0;
    outputBuffer[3] = mp.b;
    // CHECK: 80.0
}

// The emitted CUDA structs must omit every empty member, whatever the spelling. A struct member
// reads "<TypeName>_N <identifier>;" (trailing semicolon). The struct declaration
// ("struct <TypeName>_N"), a constructor ("<TypeName>_N <TypeName>_xNNinit_0("), and a pointer
// parameter ("<TypeName>_N * inner_1") all lack the "identifier;" shape, so the trailing `;` keeps
// these NOT patterns matching only a surviving member.
// CUDA-NOT: EmptyExport{{[A-Za-z0-9_]*}} {{[a-zA-Z][A-Za-z0-9_]*}};
// CUDA-NOT: EmptyPublic{{[A-Za-z0-9_]*}} {{[a-zA-Z][A-Za-z0-9_]*}};
// CUDA-NOT: EmptyNested{{[A-Za-z0-9_]*}} {{[a-zA-Z][A-Za-z0-9_]*}};
