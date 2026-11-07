//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm
//TEST:REFLECTION(filecheck=REFLECTION):-target spirv -no-codegen
//TEST:COMPILE: tests/spirv/conditional-extern-enum-override.slang -o tests/spirv/conditional-extern-enum-override.slang-module
//TEST:SIMPLE(filecheck=OVERRIDE):-target spirv-asm -r tests/spirv/conditional-extern-enum-override.slang-module
//TEST:REFLECTION(filecheck=OVERRIDE_REFLECTION):-target spirv -no-codegen -r tests/spirv/conditional-extern-enum-override.slang-module

// Check SPIR-V bindings and reflection for Conditional resources controlled by an extern enum.
// The default selects enabledBuffer; linking the override selects disabledBuffer.
import "conditional-extern-enum-type";
extern static const Mode selectedMode = Mode.On;

uniform Conditional<ConstantBuffer<float>, selectedMode != Mode.Off> enabledBuffer;
uniform Conditional<ConstantBuffer<float>, selectedMode == Mode.Off> disabledBuffer;

[shader("fragment")]
float main() : SV_Target
{
    if (let value = enabledBuffer.get())
        return value;
    if (let value = disabledBuffer.get())
        return value;
    return 0;
}

// SPIRV: OpName %[[BUFFER:[A-Za-z_0-9]+]] "enabledBuffer"
// SPIRV-NOT: OpName {{.*}} "disabledBuffer"
// SPIRV: OpDecorate %[[BUFFER]] Binding 0
// SPIRV: OpDecorate %[[BUFFER]] DescriptorSet 0
// SPIRV: %[[BUFFER]] = OpVariable {{.*}} Uniform
// SPIRV: OpAccessChain {{.*}} %[[BUFFER]]
// SPIRV: OpLoad %float

// REFLECTION: "name": "enabledBuffer",
// REFLECTION-NEXT: "binding": {"kind": "descriptorTableSlot", "index": 0},
// REFLECTION-NEXT: "type": {
// REFLECTION-NEXT: "kind": "constantBuffer",
// REFLECTION: "name": "disabledBuffer",
// REFLECTION-NEXT: "type": {
// REFLECTION-NEXT: "kind": "struct",
// REFLECTION-NEXT: "name": "Conditional",
// REFLECTION-NEXT: "fields": [
// REFLECTION-EMPTY:
// REFLECTION-NEXT: ]

// OVERRIDE: OpName %[[BUFFER:[A-Za-z_0-9]+]] "disabledBuffer"
// OVERRIDE: OpDecorate %[[BUFFER]] Binding 0
// OVERRIDE: OpDecorate %[[BUFFER]] DescriptorSet 0
// OVERRIDE: %[[BUFFER]] = OpVariable {{.*}} Uniform
// OVERRIDE: OpAccessChain {{.*}} %[[BUFFER]]
// OVERRIDE: OpLoad %float

// OVERRIDE_REFLECTION: "name": "enabledBuffer",
// OVERRIDE_REFLECTION-NEXT: "type": {
// OVERRIDE_REFLECTION-NEXT: "kind": "struct",
// OVERRIDE_REFLECTION-NEXT: "name": "Conditional",
// OVERRIDE_REFLECTION-NEXT: "fields": [
// OVERRIDE_REFLECTION-EMPTY:
// OVERRIDE_REFLECTION-NEXT: ]
// OVERRIDE_REFLECTION: "name": "disabledBuffer",
// OVERRIDE_REFLECTION-NEXT: "binding": {"kind": "descriptorTableSlot", "index": 0},
// OVERRIDE_REFLECTION-NEXT: "type": {
// OVERRIDE_REFLECTION-NEXT: "kind": "constantBuffer",
