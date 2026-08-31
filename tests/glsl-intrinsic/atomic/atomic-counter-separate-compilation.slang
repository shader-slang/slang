// A serialized GLSL module retains `GLSLAtomicUintType`, but not a front-end `-allow-glsl` policy
// switch. The final link must therefore recognize the IR type itself and replace it with the
// storage-buffer representation before direct SPIR-V emission.

//TEST:COMPILE: -lang glsl -Wno-118 tests/glsl-intrinsic/atomic/atomic-counter-separate-compilation-module.slang -o tests/glsl-intrinsic/atomic/atomic-counter-separate-compilation-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/glsl-intrinsic/atomic/atomic-counter-separate-compilation-module.slang-module -target spirv-assembly -emit-spirv-directly -stage compute -entry main

import "atomic-counter-separate-compilation-module";

[numthreads(1, 1, 1)]
void main()
{
    // Pull the separately compiled function and its atomic-counter global into the final link.
    incrementSeparatelyCompiledCounter();
}

// The counter itself, rather than only the driver's output buffer, must use the legalized storage-
// buffer representation.
// CHECK: %counter = OpVariable {{.*}} StorageBuffer
// CHECK: OpAtomicIIncrement
