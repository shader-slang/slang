//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#callback-parsed-modifiers-some-take-arguments
//META: doc_section_digest=a48e55cd5ae68644046c4a9b6fb64c4c695a77cce176cbfeb3dcf8d2ad253635
//META: purpose=`hitAttributeEXT` is a callback-parsed modifier in the raytracing family; an HLSL-style intersection shader that calls `ReportHit` with a user-defined hit-attribute struct lowers to GLSL with a `hitAttributeEXT`-decorated variable and pulls in the `GL_EXT_ray_tracing` extension.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `hitAttributeEXT` modifier keyword, whose parse
// callback is `parseHitAttributeEXTModifier`, is bound to the
// raytracing hit-attribute role. The user-facing observation is to write a `[shader("intersection")]` entry
// that uses `ReportHit` with a custom attribute struct. Slang lowers
// the HLSL-style surface into a GLSL emit that requires the
// `GL_EXT_ray_tracing` extension and binds the custom struct as a
// `hitAttributeEXT` storage-class variable. We pin both signals on
// the GLSL emit, and the SPIR-V `HitAttribute` storage class on the
// spirv-asm emit.
//
// HLSL is the third feasible target: it is the source flavour of this
// surface, so the intersection entry point and the `ReportHit` call
// carrying the user-defined attribute struct are re-emitted as-is,
// which is what shows the modifier family did not perturb the
// HLSL-flavoured round trip. Metal and WGSL have no raytracing
// capability at all and reject the entry point; see the
// `unsupported-on-target` rows in the bundle README.

//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage intersection
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage intersection -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage intersection

struct SphereHitAttributes
{
    float3 normal;
};

void main()
{
    // Use ObjectRayOrigin / RayTMin / RayTCurrent — raytracing
    // builtins available in intersection stage — and ReportHit with
    // a custom hit-attribute struct.
    float3 o = ObjectRayOrigin();
    float t = RayTMin() + RayTCurrent();

    SphereHitAttributes attrs;
    attrs.normal = o;

    ReportHit(t, 0, attrs);
}

// CHECK_GLSL: GL_EXT_ray_tracing
// CHECK_GLSL: hitAttributeEXT
// CHECK_SPV: OpEntryPoint Intersection{{NV|KHR}} %main "main"
// CHECK_SPV: HitAttribute{{NV|KHR}}

// CHECK_HLSL-DAG: struct SphereHitAttributes
// CHECK_HLSL-DAG: [shader("intersection")]
// CHECK_HLSL-DAG: ReportHit(
