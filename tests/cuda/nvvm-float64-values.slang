// Exercises scalar Float64 helper transport, arithmetic, comparison, conversions, selection, and
// same-width bit reinterpretation through the generic direct-NVVM value-operation family.

//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

RWStructuredBuffer<uint64_t> outputBuffer;

double transformDouble(double x, int64_t y)
{
    double value = ((-x + double(y)) * 3.0 - 1.0) / 2.0;
    double remainder = value % 5.0;
    double selected = select(remainder > x, remainder, x);
    int64_t integral = int64_t(selected);
    float narrowed = float(selected);
    return double(narrowed) + double(integral);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    double source = double(dispatchThreadID.x) + 1.0;
    double transformed = transformDouble(source, 4);
    outputBuffer[0] = bit_cast<uint64_t>(transformed);
    outputBuffer[1] = bit_cast<uint64_t>(bit_cast<int64_t>(3.0));
}

// CHECK: .visible .entry computeMain
// CHECK-DAG: cvt.rn.f64.u32
// CHECK-DAG: add.f64
// CHECK-DAG: mul.f64
// CHECK-DAG: div.rn.f64
// CHECK-DAG: setp.gt.f64
// CHECK-DAG: selp.f64
// CHECK-DAG: cvt.rzi.s64.f64
// CHECK-DAG: cvt.rn.f32.f64
// CHECK-DAG: cvt.f64.f32
// CHECK-DAG: st.global.u64
