//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=A `type_param` used as the element type of a global shader parameter survives front-end checking and is rejected by the IR specialization pass with E38207, reported once at the `type_param` declaration.
//META: intent=characterization
//META: covers=source/slang/slang-ir-specialize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes `SpecializationContext::specializeGlobalGenericParameters` in
// `slang-ir-specialize.cpp`. That routine runs after the specialization work
// list has drained; any `IRGlobalGenericParam` that still has uses means the
// program depends on a global generic parameter that was never bound to a
// concrete type, and it emits `UnspecializedGlobalGenericParamWithUses`
// (E38207) before deleting the parameter.
//
// The hand-written coverage of E38207 (`tests/bugs/11316-type-param-method-
// dispatch.slang`) uses the parameter through a *local* variable --
// `T t; t.get();` -- which the front end already rejects, so the IR pass never
// sees the leftover use. Declaring the parameter as the element type of a
// global shader parameter instead (`ConstantBuffer<TParam> cb;`) is accepted
// by the checker, because a global-scope `type_param` is legitimate for
// reflection and for host-side specialization; only when nothing supplies a
// binding and codegen is requested does the IR pass find the dangling use.
// That is the reporting loop this test drives.
//
// Two properties are worth noting in the observed output. The diagnostic is
// attributed to the `type_param` *declaration*, not to the `cb.get()` use
// site, and a constrained `type_param TParam : IBase` also lowers to a
// synthesized witness-table `IRGlobalGenericParam`, yet only one message is
// produced -- the reporting loop deliberately skips witness-table params when
// a user-written param was already reported. `Impl` below exists only to make
// the interface conformable; it is never referenced, so nothing binds
// `TParam`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry computeMain -stage compute

interface IBase { float get(); }

type_param TParam : IBase;
//CHECK:   ^^^^^^ global generic parameter used in code without a concrete binding
//CHECK:   ^^^^^^ a global generic parameter ('type_param' / '__generic_value_param') cannot be used in shader code without a concrete binding

struct Impl : IBase { float v; float get() { return v; } }

ConstantBuffer<TParam> cb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = cb.get();
}
