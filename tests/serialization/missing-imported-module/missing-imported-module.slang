// A precompiled module whose dependency cannot be found must fail to load with a diagnostic, not
// crash (#13343).
//
// `impl.slang-module` is built with a search path that finds `lib/dist.slang`, and loaded
// without one. A serialized module's dependencies are looked up relative to the `.slang-module`
// file itself (here `lib/lib/dist.slang`) and then on the search paths, so importing `lib/dist`
// while reading it fails. Both requirement keys of `Impl`'s witness table are then unresolvable:
// if the reader returned null for them, the second null key would collide in the table's
// `Decl*`-keyed requirement dictionary.

//TEST:COMPILE: tests/serialization/missing-imported-module/lib/impl.slang -I tests/serialization/missing-imported-module -o tests/serialization/missing-imported-module/lib/impl.slang-module
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// CHECK: result code = -1
// CHECK-NOT: {{[Ii]}}nternal
// CHECK: error[E15301]: import file not found
// CHECK-NOT: {{[Ii]}}nternal

import lib.impl;

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main()
{
    Impl impl;
    output[0] = impl.sample();
}
