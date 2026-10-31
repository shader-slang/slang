// A precompiled module whose member function signatures name a type that its dependency no
// longer exports under the recorded mangled name must fail to load with a diagnostic, not crash
// (#13343).
//
// Unlike `unresolved-imported-decl`, there is no interface here: the unresolvable reference is
// the parameter type `Rng`, which is read while `impl.slang-module` is loaded because entry-point
// discovery walks the members of `Impl`. The stale mangled name comes from the module-naming
// behavior tracked in #13343: importing `lib.rng` before `lib.impl` names the file `lib/rng`,
// while `impl.slang-module` recorded `lib/rng.slang`. If that naming changes, this test needs
// another way to produce an unresolvable imported declaration.

//TEST:COMPILE: tests/serialization/unresolved-imported-param-type/lib/impl.slang -I tests/serialization/unresolved-imported-param-type -o tests/serialization/unresolved-imported-param-type/lib/impl.slang-module
//TEST:SIMPLE(filecheck=CHECK): -I tests/serialization/unresolved-imported-param-type -target spirv-asm -entry main -stage compute

// CHECK: result code = -1
// CHECK-NOT: {{[Ii]}}nternal
// CHECK: error[E15303]: cannot resolve imported declaration '{{.*}}Rng{{.*}}'
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
