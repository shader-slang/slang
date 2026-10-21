//TEST:SIMPLE(filecheck=GENERIC): -experimental-feature -entry MyHit<ExternalContext> -stage closesthit -dump-ir -no-codegen
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry HitWithExternalContext -stage closesthit -dump-ir -no-codegen

import slang.raytracing;

// Another module supplies this context's payload, record, and primitive at link time.
// Selecting MyHit<ExternalContext> must preserve the unresolved conformance in IR;
// it cannot require concrete ABI types while compiling this module.
extern struct ExternalContext : rt::IHitContext;

// The ordinary generic function carries C and its conformance as parameters. Its input type
// uses those same parameters, so compiling MyHit does not need to know C's associated types.
// GENERIC: generic [[INPUT:%[0-9]+]] : Type
// GENERIC: [nameHint("rt.ClosestHitInput")]
// GENERIC: generic {{%[0-9]+}} : {{%[0-9]+}}
// GENERIC: param [[CONTEXT:%[A-Za-z0-9_]+]] : type_t,
// GENERIC-NEXT: param [[CONTEXT_WITNESS:%[0-9]+]] : witness_table_t({{.*}})):
// GENERIC: [nameHint("MyHit.invoke")]
// GENERIC-NEXT: func [[INVOKE:%[A-Za-z0-9_]+]] : Func(Void, specialize([[INPUT]], [[CONTEXT]], [[CONTEXT_WITNESS]]))
// GENERIC: param {{.*}} : specialize([[INPUT]], [[CONTEXT]], [[CONTEXT_WITNESS]])):
// GENERIC: return_val([[INVOKE]])
struct MyHit<C : rt::IHitContext> : rt::IClosestHitShader
{
    typealias Context = C;

    static void invoke(in rt::ClosestHitInput<Context> input) {}
}

// The ordinary input type carries both the external context and its checked conformance.
// Those arguments remain available until linking supplies the context's concrete definition.
// IR: witness_table [[CONTEXT_WITNESS:%[0-9]+]] : witness_table_t({{.*}})(%ExternalContext);
// IR: [nameHint("HitWithExternalContext.invoke")]
// IR: func {{.*}} : Func(Void, specialize({{.*}}, %ExternalContext, [[CONTEXT_WITNESS]]))
// IR: param {{.*}} : specialize({{.*}}, %ExternalContext, [[CONTEXT_WITNESS]])):
struct HitWithExternalContext : rt::IClosestHitShader
{
    typealias Context = ExternalContext;

    static void invoke(in rt::ClosestHitInput<Context> input) {}
}
