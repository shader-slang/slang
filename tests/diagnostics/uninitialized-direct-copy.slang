//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

RWStructuredBuffer<float> output;
RWStructuredBuffer<Atomic<uint> > atomicOut;

struct SelfRef
{
    SelfRef* next;
}

// Case 1 (the core bug): an uninitialized scalar is copied into a local and the
// local is then read. The checker reports at the *surviving use* of the
// propagated value -- the `output[0] = copy;` store below -- not at the
// `float copy = uninit;` initializer (a copy into a local that is never read is
// a dead store and is correctly not diagnosed). Before the fix the surviving
// store was elided so no warning was produced; now the uninitialized read is
// diagnosed at that store.
void direct_copy_to_local()
{
    float uninit;
    float copy = uninit;
    output[0] = copy;
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'uninit'
}

void direct_copy_to_buffer()
{
    float uninit;
    output[1] = uninit;
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'uninit'
}

// Case 2: a direct copy of an uninitialized scalar local into an `out`
// parameter and into an `inout` parameter.
void copy_to_out_param(out float o)
{
    float uninit;
    o = uninit;
//CHK:^ use of uninitialized variable
//CHK:^ use of uninitialized variable 'uninit'
}

void copy_to_inout_param(inout float io)
{
    float uninit;
    io = uninit;
//CHK: ^ use of uninitialized variable
//CHK: ^ use of uninitialized variable 'uninit'
}

// Case 3 (positive control): an uninitialized local used in an expression --
// this was already warned before the fix.
float use_in_expression()
{
    float uninit;
    return uninit + 1.0;
//CHK:            ^ use of uninitialized variable
//CHK:            ^ use of uninitialized variable 'uninit'
}

// Case 4: a swizzled store reads its source operand too, so copying an
// uninitialized value into swizzled components is also diagnosed (exercises
// the `SwizzledStore` arm of the store-family classification, not just `store`).
void swizzle_copy()
{
    float2 uninit;
    float4 v = float4(0.0, 0.0, 0.0, 0.0);
    v.xy = uninit;
//CHK:   ^ use of uninitialized variable
//CHK:   ^ use of uninitialized variable 'uninit'
    output[2] = v.x + v.y;
}

// Case 5: a direct copy of an uninitialized value into an atomic location.
// A direct assignment to an `Atomic<T>` lvalue lowers to `atomicStore`, whose
// value operand is likewise read; this exercises the `AtomicStore` arm of the
// store-family classification.
void copy_to_atomic()
{
    uint uninit;
    atomicOut[0] = uninit;
//CHK:           ^ use of uninitialized variable
//CHK:           ^ use of uninitialized variable 'uninit'
}

// Case 6: a copy of an uninitialized value into multiple matrix components via
// a matrix swizzle lowers to `matrixSwizzleStore`, whose source operand is read
// too; this exercises the `MatrixSwizzleStore` arm of the store-family
// classification (the final one not covered above).
void matrix_swizzle_copy()
{
    float uninit;
    float2x2 m = float2x2(0.0, 0.0, 0.0, 0.0);
    m._m00_m11 = uninit;
//CHK:           ^^^^^^ use of uninitialized variable
//CHK:           ^^^^^^ use of uninitialized variable 'uninit'
    output[3] = m[0][0] + m[1][1];
}

// Case 7 (negative control): storing the *address* of a variable is not a read
// of its contents. `s.next = &s;` lowers to `store(getFieldAddr(s), s)`, where
// the value operand is the `s` pointer itself; taking an address does not read
// the (still-uninitialized) memory, so no warning must be produced here. This
// guards against over-broadening the store-family value-operand reclassification
// (the exhaustive `diag=CHK` matching fails if any unannotated warning appears).
void store_address_not_a_read()
{
    SelfRef s;
    s.next = &s;
    output[4] = 0.0;
}

// Case 8: a direct copy of an uninitialized aggregate (struct) value into an
// `out` parameter. The stored value is a non-pointer `IRUndefined` aggregate in
// operand 1, so the value-operand reclassification applies to aggregates too,
// not just scalars/vectors/swizzles -- the uninitialized copy is diagnosed.
struct Agg
{
    float a;
    float b;
}

void copy_aggregate_to_out_param(out Agg o)
{
    Agg uninit;
    o = uninit;
//CHK:^ use of uninitialized variable
//CHK:^ use of uninitialized variable 'uninit'
}

// Case 9 (second classifier consumer): a constructor that copies an
// uninitialized aggregate into a field exercises the constructor field-init
// analysis (`checkConstructor` -> `checkFieldsFromExit` -> `isDirectlyWrittenTo`),
// which shares `getInstructionUsageType` with the direct-copy checker above. The
// uninitialized source `u` is diagnosed; the field `inner` is written (with the
// undefined value) so it is *not* reported as an uninitialized field. This pins
// the non-pointer undef-aggregate-as-store-value path in the second consumer,
// which the realistic IRVar-origin path (covered by tests/bugs/gh-6331.slang)
// does not directly exercise.
struct Outer
{
    Agg inner;
    __init()
    {
        Agg u;
        inner = u;
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'u'
    }
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    Outer outer;
    output[5] = outer.inner.a;
}
