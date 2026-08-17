// Regression test for issue #10209: an *unspecialized* generic entry point
// used to crash the compiler at IR link time ("expected entry point to be a
// function") because it lowered to an IRGeneric instead of an IRFunc. It must
// now be rejected with a clear diagnostic (E38014).
//
// A generic entry point that *is* specialized stays valid; those paths are
// covered elsewhere:
//   - `-specialize` args:        tests/front-end/generic-workgroup-size.slang
//   - generic args in the name:  tests/fp-denormal-mode/denorm-mode-generic.slang
//                                (`-entry "genericEntry<float>"`)
// The rejection here fires only when the generic is left *unspecialized*.
//
// Two directives cover both rejection paths in
// createSpecializedGlobalAndEntryPointsComponentType: the first lists the
// entry point explicitly (`-entry main`), exercising the specified-entry-point
// loop; the second omits `-entry` so `main` is discovered solely via its
// `[shader("compute")]` attribute, exercising the found-entry-point loop.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

static T scale_sample<T>(Texture3D<T> texture, int32_t3 coord)
    where T : ITexelElement, IFloat
{
    let value = texture.Load(int32_t4(coord, 0));
    return value * T(0.1F);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main<T>(
/*CHECK:
     ^^^^ E38014
     ^^^^ generic entry point 'main' must be specialized with concrete generic arguments
*/
    uint32_t3 id : SV_DispatchThreadID,
    Texture3D<T> inputTexture,
    WTexture3D<T> outputTexture) where T : ITexelElement, IFloat
{
    let scaled = scale_sample<T>(inputTexture, int32_t3(0, 0, 0));
    outputTexture.Store(id, scaled);
}
