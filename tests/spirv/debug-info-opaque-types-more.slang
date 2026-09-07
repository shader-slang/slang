//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly -capability cooperative_vector -capability cooperative_matrix

// These opaque builtin types do not reach the debug-info emitter as locals or as aggregate fields
// (an opaque type is not a debuggable value), so each is exercised as a [noinline] function
// parameter — the path that does force the type through NonSemantic debug info. Each must then get
// its own name/linkage OpString rather than the literal "unnamed" that a missing getTypeNameHint
// case would produce. The paired instantiations below differ only in an operand that the name must
// preserve (ray flags, cooperative-matrix use, tensor-layout clamp mode, tensor-view dimension), so
// they would collide under a lossy name and are what guards against dropping those operands.

using namespace linalg;

RaytracingAccelerationStructure accel;
RWStructuredBuffer<float> outBuf;

typealias CMacc = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;
typealias CMa = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;

[noinline]
float useRayQueryNone(RayQuery<RAY_FLAG_NONE> q, float r)
{
    return r;
}
[noinline]
float useRayQueryForceOpaque(RayQuery<RAY_FLAG_FORCE_OPAQUE> q, float r)
{
    return r;
}
[noinline]
float useCoopVec(CoopVec<float, 16> v, float r)
{
    return r;
}
[noinline]
float useCoopMatAccumulator(CMacc m, float r)
{
    return r;
}
[noinline]
float useCoopMatA(CMa m, float r)
{
    return r;
}
[noinline]
float useTensorLayoutUndefined(TensorLayout<2, CoopMatClampMode.Undefined> t, float r)
{
    return r;
}
[noinline]
float useTensorLayoutConstant(TensorLayout<2, CoopMatClampMode.Constant> t, float r)
{
    return r;
}
[noinline]
float useTensorView2(TensorView<2, false> t, float r)
{
    return r;
}
[noinline]
float useTensorView3(TensorView<3, false> t, float r)
{
    return r;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayQuery<RAY_FLAG_NONE> q0;
    RayQuery<RAY_FLAG_FORCE_OPAQUE> q1;
    CoopVec<float, 16> v = CoopVec<float, 16>(1.0f);
    CMacc macc = CMacc(0.0h);
    CMa ma = CMa(0.0h);
    TensorLayout<2, CoopMatClampMode.Undefined> tlu;
    TensorLayout<2, CoopMatClampMode.Constant> tlc;
    TensorView<2, false> tv2;
    TensorView<3, false> tv3;
    outBuf[tid.x] = useRayQueryNone(q0, 1.0) + useRayQueryForceOpaque(q1, 1.0) + useCoopVec(v, 1.0) +
                    useCoopMatAccumulator(macc, 1.0) + useCoopMatA(ma, 1.0) +
                    useTensorLayoutUndefined(tlu, 1.0) + useTensorLayoutConstant(tlc, 1.0) +
                    useTensorView2(tv2, 1.0) + useTensorView3(tv3, 1.0);
}

// CHECK-DAG: %[[DEBUG_INFO_NONE:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInfoNone

// Ray flags are preserved, so the two RayQuery instantiations get distinct names.
// CHECK-DAG: %[[RQ0_NAME:[0-9]+]] = OpString "RayQuery<0>"
// CHECK-DAG: %[[RQ0_LINKAGE:[0-9]+]] = OpString "@RayQuery<0>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[RQ0_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[RQ0_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %{{[0-9]+}} = OpString "RayQuery<1>"
// CHECK-DAG: %{{[0-9]+}} = OpString "@RayQuery<1>"

// CHECK-DAG: %[[CV_NAME:[0-9]+]] = OpString "CoopVec<float,16>"
// CHECK-DAG: %[[CV_LINKAGE:[0-9]+]] = OpString "@CoopVec<float,16>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[CV_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[CV_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}

// The matrix-use operand is preserved, so the two CoopMat instantiations get distinct names.
// CHECK-DAG: %[[CM0_NAME:[0-9]+]] = OpString "CoopMat<float,3,16,16,0>"
// CHECK-DAG: %[[CM0_LINKAGE:[0-9]+]] = OpString "@CoopMat<float,3,16,16,0>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[CM0_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[CM0_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %{{[0-9]+}} = OpString "CoopMat<float,3,16,16,2>"
// CHECK-DAG: %{{[0-9]+}} = OpString "@CoopMat<float,3,16,16,2>"

// The clamp-mode operand is preserved, so the two TensorLayout instantiations get distinct names.
// CHECK-DAG: %[[TL0_NAME:[0-9]+]] = OpString "TensorLayout<2,0>"
// CHECK-DAG: %[[TL0_LINKAGE:[0-9]+]] = OpString "@TensorLayout<2,0>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[TL0_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[TL0_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %{{[0-9]+}} = OpString "TensorLayout<2,1>"
// CHECK-DAG: %{{[0-9]+}} = OpString "@TensorLayout<2,1>"

// The dimension operand is preserved, so the two TensorView instantiations get distinct names.
// CHECK-DAG: %[[TV2_NAME:[0-9]+]] = OpString "TensorView<2,false,255,255,255,255,255>"
// CHECK-DAG: %[[TV2_LINKAGE:[0-9]+]] = OpString "@TensorView<2,false,255,255,255,255,255>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[TV2_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[TV2_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %{{[0-9]+}} = OpString "TensorView<3,false,255,255,255,255,255>"
// CHECK-DAG: %{{[0-9]+}} = OpString "@TensorView<3,false,255,255,255,255,255>"
