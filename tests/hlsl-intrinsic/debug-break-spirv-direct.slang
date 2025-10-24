//TEST:SIMPLE(filecheck=CHECK_SPIRV):-stage compute -entry main -target spirv -emit-spirv-directly

// CHECK_SPIRV: OpExtInstImport "NonSemantic.DebugBreak"
// CHECK_SPIRV: OpExtInst %void {{%[0-9]+}} 1

[[vk::spirv_instruction(1, "NonSemantic.DebugBreak")]]
void __spvDebugBreak();

[ForceInline]
void __debugbreak() { __spvDebugBreak(); }

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    __debugbreak();
}

