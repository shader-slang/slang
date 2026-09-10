//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -O0

// Regression for #12807: the memberless DebugTypeComposite emitted for an opaque
// resource type must use DebugInfoNone in the Size slot (not the integer 0, which
// reads as an empty zero-size composite) and a distinct "@"-prefixed linkage name
// (not the plain source name, which can collide with a user-defined type).
//
// -O0 keeps the output deterministic: at the default optimization level the
// SPIRV-Tools legalizer inserts its own unreferenced DebugInfoNone, which would
// make a bare count of DebugInfoNone ambiguous.

Texture2D texture;
SamplerState sampler;

[shader("fragment")]
float4 main(float2 uv : TEXCOORD0)
    : SV_Target
{
    return texture.Sample(sampler, uv);
}

// The readable Name stays the plain type name, the LinkageName is "@"-prefixed.
//CHECK-DAG: %[[TEX_NAME:[0-9]+]] = OpString "Texture2D"
//CHECK-DAG: %[[TEX_LINK:[0-9]+]] = OpString "@Texture2D"
//CHECK-DAG: %[[SAMP_NAME:[0-9]+]] = OpString "SamplerState"
//CHECK-DAG: %[[SAMP_LINK:[0-9]+]] = OpString "@SamplerState"

// The opaque composites reference DebugInfoNone in the Size slot (8th operand)
// and their "@"-prefixed OpString in the LinkageName slot (7th operand). SPIR-V
// asm operands can be numeric (%4) or friendly-named (%uint_0), so the skipped
// operands match [A-Za-z0-9_]+.
//CHECK-DAG: %[[NONE:[0-9]+]] = OpExtInst %void %{{[A-Za-z0-9_]+}} DebugInfoNone
//CHECK-DAG: DebugTypeComposite %[[TEX_NAME]] %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %[[TEX_LINK]] %[[NONE]]
//CHECK-DAG: DebugTypeComposite %[[SAMP_NAME]] %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %[[SAMP_LINK]] %[[NONE]]
