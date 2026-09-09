//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=8cebf6056445131d843ba65809820ad0e5294659556653bcb7502aa0a4f723ef
//META: purpose=A RefAccessorDecl (`ref`) keeps the property assignable, and the HLSL backend compiles the assignment through it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a property carrying an explicit `ref` accessor is still an
// l-value at the HLSL backend. The shader assigns through the property and
// reads it back into an output buffer; the CHECK asserts the entry point was
// emitted, which only happens if the ref accessor legalized successfully.
//
// The Metal and SPIR-V directives carry the same claim to the two remaining
// shader text targets. Both currently fail on compiler bugs, so the file is
// listed in `_meta/expected-failures.txt`:
//   - Metal aborts with the internal error "Unknown addressspace
//     encountered" (finding
//     `declarations-refaccessor-metal-unknown-addressspace-abort`).
//   - SPIR-V declares the accessor as a value-returning function while
//     calling it as if it returned `ptr<Function,int>`, so the module fails
//     validation (finding
//     `declarations-refaccessor-spirv-invalid-funcall-return-type`).
// The CHECKs below pin the *correct* shape on purpose; they are the shape
// that must hold once the bugs are fixed, and weakening them would hide the
// bugs.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

struct Cell
{
    int _v;
    property int value
    {
        get { return _v; }
        [mutating] ref { return _v; }
    }
}

//TEST_INPUT: set outBuf = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outBuf;

//CHECK: void main
// The Metal entry-point spelling below is the one the Metal back-end really
// produces for this shader when the `ref` accessor is replaced by a `set`
// accessor, so it is the shape a fixed `ref` path has to reach as well.
// (`[[kernel]]` cannot be spelled literally: FileCheck reads `[[...]]` as a
// variable reference, so the qualifier is matched as a bare substring.)
//METAL: kernel{{.*}} void main_0(
// On SPIR-V the accessor is already *called* as a pointer-returning function
// (`OpFunctionCall %_ptr_Function_int %Cell_value_ref`) and the 13 is already
// stored through that result, so the assignment survives lowering. What is
// missing is the matching function *type*: the definition is emitted as
// `OpTypeFunction %int %_ptr_Function_Cell`, which contradicts the call site
// and makes the module invalid.
//SPIRV: %{{[0-9]+}} = OpTypeFunction %_ptr_Function_int %_ptr_Function_Cell
//SPIRV: %[[REF:[0-9]+]] = OpFunctionCall %_ptr_Function_int %Cell_value_ref
//SPIRV: OpStore %[[REF]] %int_13
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    Cell c;
    c._v = 0;
    c.value = 13;
    outBuf[0] = c.value;
}
