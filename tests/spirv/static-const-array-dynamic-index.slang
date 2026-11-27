//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O0
//TEST:SIMPLE(filecheck=CHECK0): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O1

// A `static const` array read with a dynamic index must be materialized once per
// module, not copied into a fresh function-local array at every read site
// (shader-slang/slang#14739). Inlining multiplies read sites, so per-read-site
// copies scale with the number of inlined calls.
//
// At `-O0` the table is one Private global, initialized by a single store, and
// every read is an access chain into it. At higher optimization levels the
// downstream `spirv-opt` pipeline inlines the reads and then demotes that
// global (`private-to-local`) to one function-scope copy in the entry point --
// the same end state a `static` non-const array reaches -- so the array type
// still has exactly one variable instead of one per read site.

static const float kTable[8] = {0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875};

float lookup(float x)
{
    int i = clamp(int(x), 0, 7);
    return kTable[i] + kTable[min(i + 1, 7)];
}

RWStructuredBuffer<float> gOut;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float acc = gOut[id.x];
    acc += lookup(acc + float(id.x));
    acc += lookup(acc * 2.0 + float(id.x));
    acc += lookup(acc * 3.0 + float(id.x));
    acc += lookup(acc * 4.0 + float(id.x));
    gOut[id.x] = acc;
}

// At `-O0` the array is one Private global ...
// CHECK: %_ptr_Private__arr_float_int_8 = OpTypePointer Private %_arr_float_int_8
// CHECK: {{%[_a-zA-Z0-9]+}} = OpVariable %_ptr_Private__arr_float_int_8 Private
// ... exactly one of them ...
// CHECK-NOT: OpVariable %_ptr_Private__arr_float_int_8 Private
// ... no function-scope copy anywhere ...
// CHECK-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// ... and every dynamic-index read accesses the Private global.
// CHECK: OpAccessChain %_ptr_Private_float
// A function-scope copy in any function body emitted after the first read
// must not appear either; with no following CHECK this NOT scans to the end.
// CHECK-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function

// After the `-O1` downstream inlining and `private-to-local` demotion, the
// array has exactly one variable, and all reads share it.
// CHECK0: {{%[_a-zA-Z0-9]+}} = OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECK0: OpStore
// CHECK0-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECK0: OpAccessChain %_ptr_Function_float
// CHECK0-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
