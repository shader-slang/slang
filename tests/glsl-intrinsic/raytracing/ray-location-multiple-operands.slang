// Replacing a special operand removes it from its assembly block. Both names must survive to the
// output, proving that the rewrite saved the next sibling instead of ending after the first
// removal.

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/raytracing/ray-location-multiple-operands.slang -Wno-118 -target spirv-assembly -emit-spirv-directly -stage raygeneration -entry main

layout(location = 0) rayPayloadEXT vec4 payload0;
layout(location = 1) rayPayloadEXT vec4 payload1;

void main()
{
    spirv_asm
    {
        OpName __rayPayloadFromLocation(0) "first_location_operand";
        OpName __rayPayloadFromLocation(1) "second_location_operand";
    };
}

// CHECK-DAG: OpName %payload0 "first_location_operand"
// CHECK-DAG: OpName %payload1 "second_location_operand"
