// Cross-module regression test for https://github.com/shader-slang/slang/issues/11395
//
// When the struct wrapped in a `ConstantBuffer<>` comes from a *precompiled*
// module, the member/field keys lose their source locations. This exercises
// the source-location fallback chains added for E31106 / E31107: the member
// location (E31107) falls back to the parameter-group location, and the group
// location (E31106) falls back to the first-use location. Without those
// fallbacks the warnings would be emitted with no source location at all
// (the original bug).
//
// Precompile the helper to a module, then compile this file referencing it.

//TEST:COMPILE: tests/diagnostics/parameter-group-special-type-leak-location-helper.slang -o tests/diagnostics/parameter-group-special-type-leak-location-helper.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/diagnostics/parameter-group-special-type-leak-location-helper.slang-module -target spirv -entry computeMain -stage compute -o out.spv

import "parameter-group-special-type-leak-location-helper";

// The group-level warning (E31106) must carry a valid source location. The
// parameter-group type comes from the precompiled module, so the location
// falls back to the first use, which is the `cb` declaration below.
//CHECK: warning{{.*}}E31106
//CHECK-NEXT: parameter-group-special-type-leak-location-cross-module.slang:23:
ConstantBuffer<Mixed> cb;

RWStructuredBuffer<float4> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    buf[0] = cb.color;
}

// Each leaking member emits E31107. Because the member keys come from the
// precompiled module (no source loc), the location falls back to the parameter
// group location above. The important regression check is that a valid
// location is present at all, rather than the location-less warnings in #11395.
//CHECK: warning{{.*}}E31107
//CHECK-NEXT: parameter-group-special-type-leak-location-cross-module.slang:23:
//CHECK: warning{{.*}}E31107
//CHECK-NEXT: parameter-group-special-type-leak-location-cross-module.slang:23:
