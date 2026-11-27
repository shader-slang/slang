//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK0): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O0

// A `static const` array read with a dynamic index must be materialized once per
// module as a Private-storage global, not copied into a fresh function-local
// array at every read site (shader-slang/slang#14739). Inlining multiplies read
// sites, so the per-read-site copies scale with the number of inlined calls.

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

// The array has exactly one materialization, in Private storage ...
// CHECK: %_ptr_Private__arr_float_int_8 = OpTypePointer Private %_arr_float_int_8
// CHECK: {{%[_a-zA-Z0-9]+}} = OpVariable %_ptr_Private__arr_float_int_8 Private
// CHECK-NOT: OpVariable %_ptr_Private__arr_float_int_8 Private
// ... no function-scope copy of the table anywhere ...
// CHECK-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// ... and every dynamic-index read goes through the Private global.
// CHECK: OpAccessChain %_ptr_Private_float

// CHECK0: %_ptr_Private__arr_float_int_8 = OpTypePointer Private %_arr_float_int_8
// CHECK0: {{%[_a-zA-Z0-9]+}} = OpVariable %_ptr_Private__arr_float_int_8 Private
// CHECK0-NOT: OpVariable %_ptr_Private__arr_float_int_8 Private
// CHECK0-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECK0: OpAccessChain %_ptr_Private_float
