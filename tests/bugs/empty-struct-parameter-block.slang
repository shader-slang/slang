// Regression test for shader-slang/slang#8125.
//
// An empty struct used as a member of a struct must not occupy storage in the emitted
// CPU/CUDA struct: the type-layout rules give an empty struct size 0 and skip it when
// assigning offsets, so reflection reports the following field at offset 0, but an empty
// struct still occupies 1 byte as a C++/CUDA member. Leaving it in pushes the next field
// past its reflected offset, producing a host/device layout mismatch (CUDA_ERROR_ILLEGAL_
// ADDRESS in slangpy; a SIGSEGV on the CPU target). An empty member only survives
// legalization when its type is part of the public interface, so `EmptyType` is `public`.
//
// The fix removes empty struct fields in IR and rewrites all their uses to fresh empty
// values, so this also exercises shapes that an emit-only skip of the field declaration
// would break by leaving a dangling member reference: an empty field read by value across
// a [noinline] boundary (FieldExtract), an empty field passed `inout` across that boundary
// (FieldAddress), and a struct with an empty field built via an initializer list
// (a MakeStruct). The CPU target compiles emitted C++ with debug info, which preserves
// those uses, so every shape must compile and produce the expected value.

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

// Shape 2 — an empty field read by value and passed to a [noinline] function, which keeps the
// access alive to emit (the boundary slangpy's separate-compilation flow creates).
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

// Shape 3 — a struct with an empty field constructed via an initializer list (a MakeStruct).
public struct Init
{
    public EmptyType e;
    public float v;
}

// Shape 4 — an empty field passed `inout` across a [noinline] boundary: the caller must take
// the address of the empty field, exercising the `FieldAddress` -> fresh-`Var` rewrite branch.
[noinline]
float passEmptyByRef(inout EmptyType e, float v)
{
    return v + 2.0;
}

// Shape 5 — an empty field that is NOT first (`a` before it, `b` after it): removing it must
// keep `b` accessible (it exercises removing a middle field and trimming the corresponding
// MakeStruct operand at a non-zero index).
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

    // Shape 2: by-value access of an empty field across a [noinline] boundary (FieldExtract).
    Wrapper w;
    w.e = {};
    w.v = 10.0;
    outputBuffer[1] = passEmpty(w.e, w.v);
    // CHECK: 11.0

    // Shape 3: empty field via initializer-list construction (MakeStruct).
    Init s = { {}, 5.0 };
    outputBuffer[2] = s.v;
    // CHECK: 5.0

    // Shape 4: empty field passed `inout` across a [noinline] boundary (FieldAddress).
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
// CUDA: struct Mid
// CUDA-NOT: EmptyType{{.*}} e
