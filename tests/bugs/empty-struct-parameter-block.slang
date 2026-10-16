// Regression test for shader-slang/slang#8125.
//
// An empty struct occupies 0 bytes in Slang's type-layout rules (it is skipped when
// assigning field offsets), but a C++/CUDA empty struct member still occupies 1 byte. If
// an empty member survives to CPU/CUDA source emit, the field following it lands 1 byte
// past its reflected offset, so the host binds `_result` at the reflected offset while the
// device reads it 1 byte off (CUDA_ERROR_ILLEGAL_ADDRESS in slangpy; a SIGSEGV on the CPU
// target).
//
// Empty-struct legalization already lowers an empty member to `void` and drops it, so a
// plain (non-public) empty member never reached emit. But `addLinkageDecoration` used to
// tag every `public` decl with an `IRPublicDecoration`, and `isSimpleType` in empty-struct
// legalization treats a public type as "simple" and keeps it. An empty struct declared
// `public` (as `EmptyType` is here) was therefore retained and reproduced the mismatch. The
// fix stops emitting `IRPublicDecoration` for the `public` modifier, so a public empty
// struct now legalizes identically to a non-public one.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -shaderobj
// The reported failure is on CUDA (a distinct emitter); assert at the text-emit level that
// the empty members are gone from the emitted CUDA structs (no GPU needed).
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -line-directive-mode none

public struct EmptyType
{
}

// Shape 1 — the reported crash: an empty member preceding a buffer in a ParameterBlock element.
public struct CallData
{
    public EmptyType p;
    public RWStructuredBuffer<float> _result;
}

//TEST_INPUT: set call_data = new{ out ubuffer(data=[0 0], stride=4) }
ParameterBlock<CallData> call_data;

// Shape 2 — an empty field read by value and passed to a [noinline] function. The [noinline]
// keeps a function boundary that consumes the empty field, the kind slangpy's separate-
// compilation flow creates, rather than letting it inline away before legalization.
public struct Wrapper
{
    public EmptyType e;
    public float v;
}

[noinline]
float passEmpty(EmptyType e, float v)
{
    return v + 1.0;
}

// Shape 3 — a struct with an empty field constructed via an initializer list.
public struct Init
{
    public EmptyType e;
    public float v;
}

// Shape 4 — an empty field passed `inout` across a [noinline] boundary: the caller takes the
// address of the empty field, so the empty type must legalize away on an l-value path too.
[noinline]
float passEmptyByRef(inout EmptyType e, float v)
{
    return v + 2.0;
}

// Shape 5 — an empty field that is NOT first (`a` before it, `b` after it): dropping the empty
// member must keep `b` at a correct offset so it still reads back.
public struct Mid
{
    public float a;
    public EmptyType e;
    public float b;
}

// Results are read back through this top-level buffer (a buffer nested in a ParameterBlock
// dumps as hex under filecheck-buffer; a top-level buffer dumps as decimal).
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Shape 1: write through the buffer following the empty member; its offset must be 0.
    call_data._result[0] = 7.0;
    outputBuffer[0] = call_data._result[0];
    // CHECK: 7.0

    // Shape 2: by-value access of an empty field passed across a [noinline] boundary.
    Wrapper w;
    w.e = {};
    w.v = 10.0;
    outputBuffer[1] = passEmpty(w.e, w.v);
    // CHECK: 11.0

    // Shape 3: empty field in a struct built via an initializer list.
    Init s = { {}, 5.0 };
    outputBuffer[2] = s.v;
    // CHECK: 5.0

    // Shape 4: empty field passed `inout` across a [noinline] boundary (an l-value path).
    Wrapper w2;
    w2.e = {};
    w2.v = 20.0;
    outputBuffer[3] = passEmptyByRef(w2.e, w2.v);
    // CHECK: 22.0

    // Shape 5: empty field between two real fields; `b` must still read back.
    Mid m;
    m.a = 30.0;
    m.e = {};
    m.b = 40.0;
    outputBuffer[4] = m.b;
    // CHECK: 40.0
}

// The emitted CUDA structs must omit the empty members entirely.
// CUDA: struct CallData
// CUDA-NOT: EmptyType{{.*}} p
