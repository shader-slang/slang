//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main -experimental-feature
import experimental.workgraph;

// Test that [MaxRecords(n)] is encoded in dxil and the shader compiles to spirv.

struct MyRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main([MaxRecords(4)] NodeOutput<MyRecord> output, RWBuffer<int> buf)
{
    buf[0] = 0;
}

//CHK_DXIL: define void @main()
//CHK_DXIL: !{i32 1, i32 6, i32 2, !{{[0-9]+}}, i32 3, i32 4, i32 0, !{{[0-9]+}}}
//CHK_SPIRV: OpFunction
