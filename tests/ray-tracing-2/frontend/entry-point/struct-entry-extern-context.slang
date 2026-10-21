//TEST:SIMPLE: -experimental-feature -entry MyHit<ExternalContext> -stage closesthit -no-codegen
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry HitWithExternalContext -stage closesthit -dump-ir -no-codegen

import slang.raytracing;

// Another module supplies this context's payload, record, and primitive at link time.
// Selecting MyHit<ExternalContext> must preserve the unresolved conformance in IR;
// it cannot require concrete ABI types while compiling this module.
extern struct ExternalContext : rt::IHitContext;

struct MyHit<C : rt::IHitContext> : rt::IClosestHitShader
{
    typealias Context = C;

    static void invoke(in rt::ClosestHitInput<Context> input) {}
}

// A direct entry has the same unresolved context. Its IR metadata must retain the checked
// conformance, so the external context's eventual definition can supply the native ABI types.
// IR: witness_table [[CONTEXT_WITNESS:%[0-9]+]] : witness_table_t({{.*}})(%ExternalContext);
// IR: [structuralRayTracingEntryPointInfo([[STAGE_WITNESS:%[0-9]+]])]
// IR: func {{.*}} : Func(Void, specialize({{.*}}, %ExternalContext, [[CONTEXT_WITNESS]]))
// IR: witness_table [[STAGE_WITNESS]] : witness_table_t({{.*}})(%HitWithExternalContext)
// IR: witness_table_entry({{.*Context}},%ExternalContext)
// IR: witness_table_entry({{.*}},[[CONTEXT_WITNESS]])
struct HitWithExternalContext : rt::IClosestHitShader
{
    typealias Context = ExternalContext;

    static void invoke(in rt::ClosestHitInput<Context> input) {}
}
