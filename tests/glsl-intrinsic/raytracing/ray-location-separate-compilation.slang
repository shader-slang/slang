// The GLSL overload lowers the final payload argument to a ray-location assembly operand. After
// serialization and reload, the final direct-SPIR-V pass must still resolve that operand to the
// linked global carrying `layout(location = 2)`.

//TEST:COMPILE: -lang glsl -Wno-118 tests/glsl-intrinsic/raytracing/ray-location-separate-compilation-module.slang -o tests/glsl-intrinsic/raytracing/ray-location-separate-compilation-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/glsl-intrinsic/raytracing/ray-location-separate-compilation-module.slang-module -target spirv-assembly -emit-spirv-directly -stage raygeneration -entry main

import "ray-location-separate-compilation-module";

[shader("raygeneration")]
void main()
{
    // Pull the serialized GLSL call and its location-decorated payload into the final link.
    traceWithSeparatelyCompiledPayload();
}

// Resolving the location operand replaces it with the linked payload global before emission.
// CHECK: OpTraceRayKHR {{.*}} %payload
