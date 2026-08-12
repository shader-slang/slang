//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=ByteAddressBuffer and RWByteAddressBuffer uniforms lower to global_params of the same-spelled IR types (no element-type operand).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ByteAddressBuffer and RWByteAddressBuffer uniforms lower to
// global_params of the same-spelled IR types (no element-type
// operand). Both uniforms surface as bare-named IR types on their
// respective `global_param` instructions.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ByteAddressBuffer src;
uniform RWByteAddressBuffer dst;

[numthreads(1, 1, 1)]
void main()
{
    dst.Store(0, src.Load(0));
}

// IR: let {{.*}}%src{{.*}}: ByteAddressBuffer{{.*}}= global_param
// IR: let {{.*}}%dst{{.*}}: RWByteAddressBuffer{{.*}}= global_param
