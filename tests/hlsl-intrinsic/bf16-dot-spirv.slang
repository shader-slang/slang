//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

// Verify that dot(vector<BFloat16, N>, vector<BFloat16, N>) lowers to OpDot
// under the BFloat16DotProductKHR capability (SPV_KHR_bfloat16). Per the
// extension spec, OpDot on bfloat16 vectors is one of the few permitted
// operations on the bfloat16 type.

//SPV: OpCapability BFloat16TypeKHR
//SPV: OpCapability BFloat16DotProductKHR
//SPV: OpExtension "SPV_KHR_bfloat16"

RWStructuredBuffer<BFloat16> outBuf;

[numthreads(1,1,1)]
void computeMain()
{
    vector<BFloat16, 2> a2 = vector<BFloat16, 2>(BFloat16(1.0), BFloat16(2.0));
    vector<BFloat16, 2> b2 = vector<BFloat16, 2>(BFloat16(0.5), BFloat16(0.5));

    vector<BFloat16, 3> a3 = vector<BFloat16, 3>(BFloat16(1.0), BFloat16(2.0), BFloat16(3.0));
    vector<BFloat16, 3> b3 = vector<BFloat16, 3>(BFloat16(0.5), BFloat16(0.5), BFloat16(1.0));

    vector<BFloat16, 4> a4 = vector<BFloat16, 4>(BFloat16(1.0), BFloat16(2.0), BFloat16(3.0), BFloat16(4.0));
    vector<BFloat16, 4> b4 = vector<BFloat16, 4>(BFloat16(0.5), BFloat16(0.5), BFloat16(1.0), BFloat16(1.0));

    //SPV-DAG: OpDot %bfloat16
    //SPV-DAG: OpDot %bfloat16
    //SPV-DAG: OpDot %bfloat16
    outBuf[0] = dot(a2, b2);
    outBuf[1] = dot(a3, b3);
    outBuf[2] = dot(a4, b4);
}
