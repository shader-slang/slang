// A precompiled module that refers to declarations its dependency no longer exports under the
// recorded mangled names must fail to load with a diagnostic, not crash (#13343).
//
// Importing `lib.rng` before `lib.impl` names that module `lib/rng`, so when `lib/dist` is
// compiled from source here, `IDist`'s requirements mangle differently from what
// `impl.slang-module` recorded. Both requirement keys of `Impl`'s witness table fail to resolve;
// loading stops at the first one.

//TEST:COMPILE: tests/serialization/unresolved-imported-decl/lib/impl.slang -I tests/serialization/unresolved-imported-decl -o tests/serialization/unresolved-imported-decl/lib/impl.slang-module
//TEST:SIMPLE(filecheck=CHECK): -I tests/serialization/unresolved-imported-decl -target spirv-asm -entry main -stage compute

// CHECK: result code = -1
// CHECK-NOT: {{[Ii]}}nternal
// CHECK: error[E15303]: cannot resolve imported declaration '{{.*}}IDist{{.*}}'
// CHECK-NOT: E15303
// CHECK-NOT: {{[Ii]}}nternal

import lib.rng;
import lib.impl;

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main()
{
    Rng rng = { 0 };
    Impl impl;
    output[0] = impl.sample(rng);
}
