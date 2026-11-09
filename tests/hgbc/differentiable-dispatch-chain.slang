//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct FloatBuffer : IDifferentiablePtrType
{
    typealias Differential = FloatBuffer;

    public RWStructuredBuffer<float> buffer;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void scaleKernel(uniform FloatBuffer x, uniform FloatBuffer y)
{
    y.buffer[0] = 2.0f * x.buffer[0];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void biasKernel(uniform FloatBuffer x, uniform FloatBuffer y)
{
    y.buffer[0] = x.buffer[0] + 3.0f;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void scaleBwdKernel(
    uniform FloatBuffer x,
    uniform FloatBuffer dx,
    uniform FloatBuffer y,
    uniform FloatBuffer dy)
{
    dx.buffer[0] += 2.0f * dy.buffer[0];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void biasBwdKernel(
    uniform FloatBuffer x,
    uniform FloatBuffer dx,
    uniform FloatBuffer y,
    uniform FloatBuffer dy)
{
    dx.buffer[0] += dy.buffer[0];
}

void scale_bwd(DifferentialPtrPair<FloatBuffer> x, DifferentialPtrPair<FloatBuffer> y)
{
    dispatch(scaleBwdKernel, uint3(1, 1, 1))(x.p, x.d, y.p, y.d);
}

[BackwardDerivative(scale_bwd)]
void scale(FloatBuffer x, FloatBuffer y)
{
    dispatch(scaleKernel, uint3(1, 1, 1))(x, y);
}

void bias_bwd(DifferentialPtrPair<FloatBuffer> x, DifferentialPtrPair<FloatBuffer> y)
{
    dispatch(biasBwdKernel, uint3(1, 1, 1))(x.p, x.d, y.p, y.d);
}

[BackwardDerivative(bias_bwd)]
void bias(FloatBuffer x, FloatBuffer y)
{
    dispatch(biasKernel, uint3(1, 1, 1))(x, y);
}

[Differentiable]
void runChain(FloatBuffer x, FloatBuffer tmp, FloatBuffer y)
{
    scale(x, tmp);
    bias(tmp, y);
}

[host]
public void main(
    FloatBuffer x,
    FloatBuffer dx,
    FloatBuffer tmp,
    FloatBuffer dtmp,
    FloatBuffer y,
    FloatBuffer dy)
{
    runChain(x, tmp, y);

    let xPair = DifferentialPtrPair<FloatBuffer>(x, dx);
    let tmpPair = DifferentialPtrPair<FloatBuffer>(tmp, dtmp);
    let yPair = DifferentialPtrPair<FloatBuffer>(y, dy);
    bwd_diff(runChain)(xPair, tmpPair, yPair);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_FLOAT_BUFFER:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 11 "FloatBuffer"
// CHECK-DAG: [[STR_SCALE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 11 "scaleKernel"
// CHECK-DAG: [[STR_BIAS:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 10 "biasKernel"
// CHECK-DAG: [[STR_SCALE_BWD:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 14 "scaleBwdKernel"
// CHECK-DAG: [[STR_BIAS_BWD:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 13 "biasBwdKernel"

// CHECK: Section("EXPT")
// CHECK-DAG: OpExport Function {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}
// CHECK-DAG: OpExport Type [[STR_FLOAT_BUFFER]] [[FLOAT_BUFFER:%[A-Za-z0-9_]+]]

// CHECK: Section("TYPE")
// CHECK-DAG: [[FLOAT:%[A-Za-z0-9_]+]] = OpFloat32Type
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[FLOAT]]
// CHECK-DAG: [[FLOAT_BUFFER]] = OpStructType {{%[A-Za-z0-9_]+}} [[BUFFER]]

// CHECK: Section("FUNC")
// CHECK-DAG: [[SCALE_DISPATCH_SIZE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpMakeVector
// CHECK-DAG: OpCall [[SCALE_DISPATCHER:%[A-Za-z0-9_]+]] [[SCALE_DISPATCH_SIZE]]
// CHECK-DAG: [[BIAS_DISPATCH_SIZE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpMakeVector
// CHECK-DAG: OpCall [[BIAS_DISPATCHER:%[A-Za-z0-9_]+]] [[BIAS_DISPATCH_SIZE]]
// CHECK-DAG: [[BIAS_BWD_DISPATCH_SIZE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpMakeVector
// CHECK-DAG: OpCall [[BIAS_BWD_DISPATCHER:%[A-Za-z0-9_]+]] [[BIAS_BWD_DISPATCH_SIZE]]
// CHECK-DAG: [[SCALE_BWD_DISPATCH_SIZE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpMakeVector
// CHECK-DAG: OpCall [[SCALE_BWD_DISPATCHER:%[A-Za-z0-9_]+]] [[SCALE_BWD_DISPATCH_SIZE]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpVar
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpFieldAddress {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}
// CHECK-DAG: OpStore {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpLoad {{%[A-Za-z0-9_]+}}

// CHECK: Section("EMBD")
// CHECK-DAG: [[SCALE_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint {{%[A-Za-z0-9_]+}} [[STR_SCALE]]
// CHECK-DAG: [[BIAS_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint {{%[A-Za-z0-9_]+}} [[STR_BIAS]]
// CHECK-DAG: [[SCALE_BWD_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint {{%[A-Za-z0-9_]+}} [[STR_SCALE_BWD]]
// CHECK-DAG: [[BIAS_BWD_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint {{%[A-Za-z0-9_]+}} [[STR_BIAS_BWD]]
// CHECK-DAG: [[SCALE_DISPATCHER]] : {{%[A-Za-z0-9_]+}} = OpMakeDispatcher Compute [[SCALE_ENTRY]]
// CHECK-DAG: [[BIAS_DISPATCHER]] : {{%[A-Za-z0-9_]+}} = OpMakeDispatcher Compute [[BIAS_ENTRY]]
// CHECK-DAG: [[SCALE_BWD_DISPATCHER]] : {{%[A-Za-z0-9_]+}} = OpMakeDispatcher Compute [[SCALE_BWD_ENTRY]]
// CHECK-DAG: [[BIAS_BWD_DISPATCHER]] : {{%[A-Za-z0-9_]+}} = OpMakeDispatcher Compute [[BIAS_BWD_ENTRY]]
