//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

RWStructuredBuffer<float> output;
RWStructuredBuffer<Atomic<uint> > atomicOut;
RaytracingAccelerationStructure accelerationStructure;
groupshared int returnedPointee;
groupshared int variableInitializerPointee;

typealias GroupSharedIntPointer = Ptr<int, Access::ReadWrite, AddressSpace::GroupShared>;

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

// Case 9: constructor checking uses the same instruction-effect classifier as the direct-copy
// cases above. Copying `u` reads an uninitialized aggregate and writes `inner`. We therefore
// diagnose `u` but do not also report `inner` as an uninitialized field. This case starts from an
// `IRUndefined`; `tests/bugs/gh-6331.slang` covers the usual `IRVar` form.
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

// Case 10: a `$value` operand passes an rvalue to a SPIR-V assembly instruction. The
// uninitialized-value analysis must treat that operand as a read even though an `&value` operand
// may instead be an output address.
void spirv_asm_reads_value()
{
    int uninit;
    int copy = spirv_asm
//CHK:         ^^^^^^^^^ use of uninitialized variable
//CHK:         ^^^^^^^^^ use of uninitialized variable 'uninit'
    {
        OpCopyObject $$int result $uninit
    };
    output[6] = copy;
}

// Case 11: `OpLoad` reads the value stored at its `&value` operand. The surrounding operand wrapper
// alone does not state that direction, so the analysis must use the containing SPIR-V opcode.
void spirv_asm_loads_address()
{
    int uninit;
    int copy = spirv_asm
//CHK:         ^^^^^^^^^ use of uninitialized variable
//CHK:         ^^^^^^^^^ use of uninitialized variable 'uninit'
    {
        OpLoad $$int result &uninit
    };
    output[7] = copy;
}

// Case 12: an atomic read-modify-write operation reads the value at its address before replacing
// it. The operation must therefore diagnose the first read, while its unconditional write must
// initialize the value for the load that follows.
void atomic_add_reads_then_initializes()
{
    Atomic<uint> value;
    uint previous = value.add(1);
//CHK:                       ^ use of uninitialized variable
//CHK:                       ^ use of uninitialized variable 'value'
    output[8] = previous + value.load();
}

// Case 13: an inline SPIR-V atomic read-modify-write has the same initialization effects as its
// native Slang counterpart. The analysis must recognize the pointer after the result type and
// result ID as the address being read and written.
void spirv_asm_atomic_reads_address()
{
    int value;
    int increment = 1;
    int previous = spirv_asm
//CHK:             ^^^^^^^^^ use of uninitialized variable
//CHK:             ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpAtomicIAdd $$int result &value Device Relaxed $increment
    };
    output[9] = previous + value;
}

// Case 14: `OpAtomicLoad` reads its pointer operand after the result type and result ID. It does not
// write the pointee.
void spirv_asm_atomic_load_reads_address()
{
    int value;
    int loaded = spirv_asm
//CHK:           ^^^^^^^^^ use of uninitialized variable
//CHK:           ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpAtomicLoad $$int result &value Device Relaxed
    };
    output[10] = loaded;
}

// Case 15: `OpAtomicStore` has no result type or result ID, so its pointer is operand one. Its
// unconditional write initializes the pointee for the later load.
void spirv_asm_atomic_store_initializes_address()
{
    int value;
    int replacement = 1;
    spirv_asm
    {
        OpAtomicStore &value Device Relaxed $replacement
    };
    output[11] = value;
}

// Case 16: inline-SPIR-V compare-exchange reads the old value but writes only when the comparison
// succeeds. The first use reports the read, and the conditional write cannot prove that the later
// load is initialized on every path.
void spirv_asm_compare_exchange_may_leave_value_uninitialized()
{
    int value;
    int replacement = 1;
    int comparison = 0;
    int previous = spirv_asm
//CHK:             ^^^^^^^^^ use of uninitialized variable
//CHK:             ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpAtomicCompareExchange $$int result &value Device Relaxed Relaxed $replacement $comparison
    };
    output[12] = previous + value;
//CHK:                    ^ possible use of uninitialized variable
//CHK:                    ^ variable 'value' may be uninitialized on some paths; it is only conditionally assigned
}

// Case 17: compare-exchange reads the previous value but writes only when the comparison succeeds.
// Its first use must report the read, and its conditional write cannot prove that the later load is
// initialized on every path.
void atomic_compare_exchange_may_leave_value_uninitialized()
{
    Atomic<uint> value;
    value.compareExchange(0, 1);
//CHK:                   ^ use of uninitialized variable
//CHK:                   ^ use of uninitialized variable 'value'
    output[13] = value.load();
//CHK:                     ^ possible use of uninitialized variable
//CHK:                     ^ variable 'value' may be uninitialized on some paths; it is only conditionally assigned
}

// Case 18: an extension memory operation must receive the same address-role analysis as a core
// `OpLoad`. `OpSubgroupBlockReadINTEL` therefore classifies `&value` as a read, so the checker reports
// the uninitialized value at this instruction.
void spirv_extension_memory_op_reads_address()
{
    int value;
    int loaded = spirv_asm
//CHK:           ^^^^^^^^^ use of uninitialized variable
//CHK:           ^^^^^^^^^ use of uninitialized variable 'value'
    {
        OpSubgroupBlockReadINTEL $$int result &value
    };
    output[14] = loaded;
}

// Case 19: GLSL.std.450 `Frexp` writes the exponent through its final operand. That operand is an
// output only: the operation must not read `exponent`, and it must initialize `exponent` for the
// following expression.
void spirv_extension_output_initializes_address()
{
    int exponent;
    float input = 1.0;
    float fraction = spirv_asm
    {
        result:$$float = OpExtInst glsl450 Frexp $input &exponent
    };
    output[15] = fraction + exponent;
}

// Case 20: `OpRayQueryInitializeKHR` completely initializes its ray-query operand, so a following
// getter may read that operand. The same getter must diagnose a ray-query variable that no
// initialization operation has written.
void spirv_ray_query_initialization_and_read()
{
    uint rayFlags = 0;
    uint instanceMask = 0xff;
    float3 origin = 0.0;
    float minimumT = 0.0;
    float3 direction = float3(0.0, 0.0, 1.0);
    float maximumT = 1.0;

    RayQuery<0> initializedQuery;
    spirv_asm
    {
        OpCapability RayQueryKHR;
        OpExtension "SPV_KHR_ray_query";
        OpRayQueryInitializeKHR &initializedQuery $accelerationStructure $rayFlags $instanceMask
            $origin $minimumT $direction $maximumT;
    };
    float initializedMinimumT = spirv_asm
    {
        result:$$float = OpRayQueryGetRayTMinKHR &initializedQuery
    };

    RayQuery<0> uninitializedQuery;
    float uninitializedMinimumT = spirv_asm
//CHK:                            ^^^^^^^^^ use of uninitialized variable
//CHK:                            ^^^^^^^^^ use of uninitialized variable 'uninitializedQuery'
    {
        result:$$float = OpRayQueryGetRayTMinKHR &uninitializedQuery
    };
    output[16] = initializedMinimumT + uninitializedMinimumT;
}

// Case 21: `OpHitObjectRecordEmptyEXT` completely initializes a hit object. A getter may read the
// recorded object, while the same getter must diagnose an object that has not been recorded or
// filled by a trace operation.
void spirv_hit_object_initialization_and_read()
{
    HitObject initializedHitObject;
    spirv_asm
    {
        OpExtension "SPV_EXT_shader_invocation_reorder";
        OpCapability ShaderInvocationReorderEXT;
        OpHitObjectRecordEmptyEXT &initializedHitObject;
    };
    bool initializedIsHit = spirv_asm
    {
        result:$$bool = OpHitObjectIsHitEXT &initializedHitObject
    };

    HitObject uninitializedHitObject;
    bool uninitializedIsHit = spirv_asm
//CHK:                        ^^^^^^^^^ use of uninitialized variable
//CHK:                        ^^^^^^^^^ use of uninitialized variable 'uninitializedHitObject'
    {
        result:$$bool = OpHitObjectIsHitEXT &uninitializedHitObject
    };
    output[17] = initializedIsHit == uninitializedIsHit ? 1.0 : 0.0;
}

// Case 22: the GLSL.std.450 interpolation operations read through their interpolant operand. An
// uninitialized interpolant must therefore be diagnosed at the assembly expression.
void spirv_interpolation_reads_address()
{
    float interpolant;
    float interpolated = spirv_asm
//CHK:                   ^^^^^^^^^ use of uninitialized variable
//CHK:                   ^^^^^^^^^ use of uninitialized variable 'interpolant'
    {
        result:$$float = OpExtInst glsl450 InterpolateAtCentroid &interpolant
    };
    output[18] = interpolated;
}

// Case 23: the object operand of `OpStore` is a pointer value, not an access to that pointer's
// pointee. Storing `&value` must not turn the later read of `value` into a conditional-initialization
// diagnostic.
void spirv_store_pointer_value_does_not_access_pointee()
{
    int value;
    __Addr<int> saved;
    spirv_asm
    {
        OpStore &saved &value;
    };
    output[19] = value;
//CHK:           ^^^^^ use of uninitialized variable
//CHK:           ^^^^^ use of uninitialized variable 'value'
}

// Case 24: NonSemantic.DebugPrintf formats a pointer value without dereferencing it. Passing
// `&value` must likewise leave the later read definitely uninitialized rather than treating the
// debug instruction as a possible write.
void spirv_debug_printf_pointer_does_not_access_pointee()
{
    int value;
    spirv_asm
    {
        %format = OpString "%p";
        %ignored:$$void = OpExtInst debugPrintf 1 %format &value;
    };
    output[20] = value;
//CHK:           ^^^^^ use of uninitialized variable
//CHK:           ^^^^^ use of uninitialized variable 'value'
}

// Case 25: `OpReturnValue` returns a pointer value; it does not read or write the pointer's pointee.
// The inline `OpReturnValue` terminates the emitted SPIR-V function. The following Slang return is
// unreachable after emission, but it keeps the source function well-formed. Returning
// `&returnedPointee` must not make the later read appear initialized.
GroupSharedIntPointer expose_uninitialized_address()
{
    spirv_asm
    {
        OpReturnValue &returnedPointee;
    };
    return __getAddress(returnedPointee);
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    Outer outer;
    output[5] = outer.inner.a;

    GroupSharedIntPointer exposedAddress = expose_uninitialized_address();
    output[21] = returnedPointee;
//CHK:           ^^^^^^^^^^^^^^^ use of uninitialized global variable
//CHK:           ^^^^^^^^^^^^^^^ use of uninitialized global variable 'returnedPointee'

    // Case 26: an `OpVariable` initializer copies its pointer operand into the new variable. It
    // does not access the copied pointer's pointee, so `variableInitializerPointee` remains
    // uninitialized after the assembly expression.
    spirv_asm
    {
        %slot : $$__Addr<GroupSharedIntPointer> =
            OpVariable Function &variableInitializerPointee;
    };
    output[22] = variableInitializerPointee;
//CHK:           ^^^^^^^^^^^^^^^^^^^^^^^^^^ use of uninitialized global variable
//CHK:           ^^^^^^^^^^^^^^^^^^^^^^^^^^ use of uninitialized global variable 'variableInitializerPointee'
}
