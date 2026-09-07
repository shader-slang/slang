//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly -capability cooperative_vector -capability cooperative_matrix

// The linalg cooperative/tensor-addressing types are opaque and only reach the debug-info emitter as
// function parameters (an opaque type is not a debuggable local or field). Each name must carry the
// operands that form the type's identity so distinct instantiations don't collide on the literal
// "unnamed" that a missing getTypeNameHint case would produce. The paired instantiations differ only
// in a preserved operand (cooperative-matrix use, tensor-layout clamp mode, tensor-view dimension),
// which is what guards against dropping that operand. TensorView uses valid permutation indices so
// the emitted OpTypeTensorViewNV passes SPIR-V validation.

using namespace linalg;

RWStructuredBuffer<float> outBuf;

typealias CMacc = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;
typealias CMa = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;

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
float useTensorView2(TensorView<2, false, 0, 1> t, float r)
{
    return r;
}
[noinline]
float useTensorView3(TensorView<3, false, 0, 1, 2> t, float r)
{
    return r;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    CoopVec<float, 16> v = CoopVec<float, 16>(1.0f);
    CMacc macc = CMacc(0.0h);
    CMa ma = CMa(0.0h);
    TensorLayout<2, CoopMatClampMode.Undefined> tlu;
    TensorLayout<2, CoopMatClampMode.Constant> tlc;
    TensorView<2, false, 0, 1> tv2;
    TensorView<3, false, 0, 1, 2> tv3;
    outBuf[tid.x] = useCoopVec(v, 1.0) + useCoopMatAccumulator(macc, 1.0) + useCoopMatA(ma, 1.0) +
                    useTensorLayoutUndefined(tlu, 1.0) + useTensorLayoutConstant(tlc, 1.0) +
                    useTensorView2(tv2, 1.0) + useTensorView3(tv3, 1.0);
}

// CHECK-DAG: %[[DEBUG_INFO_NONE:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInfoNone

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
// CHECK-DAG: %[[TV2_NAME:[0-9]+]] = OpString "TensorView<2,false,0,1,255,255,255>"
// CHECK-DAG: %[[TV2_LINKAGE:[0-9]+]] = OpString "@TensorView<2,false,0,1,255,255,255>"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[TV2_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[TV2_LINKAGE]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %{{[0-9]+}} = OpString "TensorView<3,false,0,1,2,255,255>"
// CHECK-DAG: %{{[0-9]+}} = OpString "@TensorView<3,false,0,1,2,255,255>"
