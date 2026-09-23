//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -emit-spirv-directly

// Regression test for shader-slang/slang#12108.
//
// The SPIR-V emitter auto-emits an OpName for every named `%id` register used in
// a spirv_asm block. When a core-module intrinsic names an internal register with
// a plain, user-plausible identifier (e.g. %dotResult), that name leaks into the
// emitted module's OpName debug section and looks like a user symbol. #12108
// prefixes every internal spirv_asm register in *.meta.slang with `__`, so its
// OpName reads unambiguously as compiler-internal.
//
// dot4add_u8packed lowers to a spirv_asm block whose intermediate register was
// %dotResult and is now %__dotResult. This checks the emitted OpName is prefixed,
// and that the old un-prefixed spelling no longer appears.

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    // CHECK: OpName %{{[A-Za-z0-9_]+}} "__dotResult"
    //
    // The bare "dotResult" name must not appear: in "__dotResult" the character
    // before `dotResult` is `_`, never `"`, so a quote-anchored negative check is
    // exact and does not match the prefixed form.
    // CHECK-NOT: "dotResult"
    outputBuffer[0] = dot4add_u8packed(0x01020304u, 0x05060708u, 10u);
}
