//TEST:COMPILE: tests/library/export-library-global.slang -o tests/library/export-library-global.slang-module -target spirv -embed-downstream-ir
//TEST:SIMPLE(filecheck=LINKED_CHECK): tests/library/precompiled-spirv-global.slang-module tests/library/export-library-global.slang-module -target spirv -dump-ir

// LINKED_CHECK: Generator: Khronos SPIR-V Tools Linker; 0

extern static int myGlobalVar;

layout(location = 0) out float4 fragColor;

[shader("fragment")]
void main() {
    fragColor = float4(myGlobalVar);
}
