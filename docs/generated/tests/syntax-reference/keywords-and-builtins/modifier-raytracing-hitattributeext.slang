//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#complex-modifiers-take-arguments
//META: doc_section_digest=80bd4844532fa1dfb157f799dd251a83773921d0d32ca7088915141e5b59809b
//META: purpose=`hitAttributeEXT` is a complex modifier in the raytracing family; an HLSL-style intersection shader that calls `ReportHit` with a user-defined hit-attribute struct lowers to GLSL with a `hitAttributeEXT`-decorated variable and pulls in the `GL_EXT_ray_tracing` extension.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `hitAttributeEXT` in the complex-modifiers table as
// `parseHitAttributeEXTModifier (raytracing)`. The cleanest user-
// facing observation is to write a `[shader("intersection")]` entry
// that uses `ReportHit` with a custom attribute struct. Slang lowers
// the HLSL-style surface into a GLSL emit that requires the
// `GL_EXT_ray_tracing` extension and binds the custom struct as a
// `hitAttributeEXT` storage-class variable. We pin both signals on
// the GLSL emit, and the SPIR-V `HitAttribute` storage class on the
// spirv-asm emit, so the same claim is observed on two backends.

//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage intersection
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage intersection -emit-spirv-directly

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
