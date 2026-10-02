// Compile both modules separately, matching issue #13276's extern/export boundary.
//TEST:COMPILE: tests/serialization/link-time-payload/payload_contract.slang -o tests/serialization/link-time-payload/payload_contract.slang-module
//TEST:COMPILE: tests/serialization/link-time-payload/payload_renderer.slang -o tests/serialization/link-time-payload/payload_renderer.slang-module
//TEST:SIMPLE_EX(filecheck=SPIRV): tests/serialization/link-time-payload/payload_contract.slang-module -entry closest_hit tests/serialization/link-time-payload/payload_renderer.slang-module -target spirv-asm -profile sm_6_6
//TEST:SIMPLE_EX(filecheck=SPIRV): tests/serialization/link-time-payload/payload_contract.slang-module -entry explicit_semantics_hit tests/serialization/link-time-payload/payload_renderer.slang-module -target spirv-asm -profile sm_6_6
//TEST:SIMPLE_EX(filecheck=CUDA): tests/serialization/link-time-payload/payload_contract.slang-module -entry closest_hit tests/serialization/link-time-payload/payload_renderer.slang-module -target cuda -profile sm_6_6
//TEST:SIMPLE_EX(filecheck=CUDA): tests/serialization/link-time-payload/payload_contract.slang-module -entry explicit_semantics_hit tests/serialization/link-time-payload/payload_renderer.slang-module -target cuda -profile sm_6_6

//TEST:SIMPLE_EX(filecheck=SPIRV): tests/serialization/link-time-payload/payload_contract.slang-module tests/serialization/link-time-payload/payload_renderer.slang-module -entry concrete_hit -target spirv-asm -profile sm_6_6
//TEST:SIMPLE_EX(filecheck=CUDA): tests/serialization/link-time-payload/payload_contract.slang-module tests/serialization/link-time-payload/payload_renderer.slang-module -entry concrete_hit -target cuda -profile sm_6_6
//TEST:SIMPLE_EX(filecheck=SPIRV): tests/serialization/link-time-payload/payload_contract.slang-module tests/serialization/link-time-payload/payload_renderer.slang-module -entry concrete_renderer_hit -target spirv-asm -profile sm_6_6
//TEST:SIMPLE_EX(filecheck=CUDA): tests/serialization/link-time-payload/payload_contract.slang-module tests/serialization/link-time-payload/payload_renderer.slang-module -entry concrete_renderer_hit -target cuda -profile sm_6_6

// SPIRV-DAG: OpTypePointer IncomingRayPayloadKHR
// SPIRV-DAG: OpTypePointer HitAttributeKHR
// SPIRV: OpFAdd
// SPIRV: OpStore
// CUDA: optixGetPayload_0
// CUDA: optixSetPayload_0
