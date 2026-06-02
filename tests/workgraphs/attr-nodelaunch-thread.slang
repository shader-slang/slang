//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
// The current SPIR-V node-stage path emits AMDX workgraph constructs without full validator support.
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main -skip-spirv-validation -experimental-feature
import experimental.workgraph;

// Test that a thread-launch node shader compiles to dxil and spirv.

[shader("node")]
[NodeLaunch("thread")]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

//CHK_DXIL: define void @main()
//CHK_SPIRV: OpFunction
