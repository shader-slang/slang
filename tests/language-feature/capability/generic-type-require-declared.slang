// A declaration with its own `[require(...)]` that names a requiring generic type
// must be checked against that type's requirement, exactly as for the non-generic
// spelling. The `NG` declarations are the non-generic twins of the ones above them
// and must report the same diagnostics.
//
// Regression test for https://github.com/shader-slang/slang/issues/13579.
//
// CHECK is non-exhaustive because E36108 repeats its "see definition of" note for
// the function (for both spellings). The unanchored CHECK lines below pin, once per
// spelling, the notes that point back at a type declaration: the "see definition of"
// note of each E36108/E36104, the "see declaration of" the interface of each E36118,
// and the "see declaration of 'get'" of each E36119. CLEAN compiles only the
// declarations whose requirements are compatible, and is exhaustive with no
// annotations, so any diagnostic there fails the test.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CLEAN): -target hlsl -no-codegen -DCLEAN_ONLY

[require(metal)]
public struct MetalOnly<let N : int>
{
    uint value;
}

[require(metal)]
public struct MetalOnlyNG
{
    uint value;
}

[require(spirv, spvAtomicFloat32AddEXT)]
public struct SpirvAtomic<let N : int>
{
    uint value;
}

[require(spirv, spvAtomicFloat32AddEXT)]
public struct SpirvAtomicNG
{
    uint value;
}

[require(metal)]
public interface IGen<T>
{
    uint get();
}

[require(metal)]
public interface ING
{
    uint get();
}

#ifndef CLEAN_ONLY

//CHECK: see definition of 'MetalOnly'
//CHECK: see definition of 'MetalOnlyNG'
//CHECK: see definition of 'SpirvAtomic'
//CHECK: see definition of 'SpirvAtomicNG'
//CHECK: see declaration of 'IGen'
//CHECK: see declaration of 'ING'
//CHECK: see declaration of 'get'
//CHECK: see declaration of 'get'

// The declared target is disjoint from the type's.
[require(hlsl)]
public uint declaredHlsl(MetalOnly<3> a)
/*CHECK:
            ^^^^^^^^^^^^ dependencies not compatible on target
                                      ^ see using of 'MetalOnly'
*/
{
    return a.value;
}

[require(hlsl)]
public uint declaredHlslNG(MetalOnlyNG a)
/*CHECK:
            ^^^^^^^^^^^^^^ dependencies not compatible on target
                                       ^ see using of 'MetalOnlyNG'
*/
{
    return a.value;
}

// The declared requirement is a strict subset of the type's.
[require(spirv)]
public uint declaredSpirv(SpirvAtomic<3> a)
/*CHECK:
            ^^^^^^^^^^^^^ 'declaredSpirv' uses undeclared capability 'spvAtomicFloat32AddEXT'
                                         ^ see using of 'SpirvAtomic'
*/
{
    return a.value;
}

[require(spirv)]
public uint declaredSpirvNG(SpirvAtomicNG a)
/*CHECK:
            ^^^^^^^^^^^^^^^ 'declaredSpirvNG' uses undeclared capability 'spvAtomicFloat32AddEXT'
                                          ^ see using of 'SpirvAtomicNG'
*/
{
    return a.value;
}

// A subtype must support every target its supertype requires. The member `get`
// inherits `hlsl` and also reports E36119 against the interface's `get`, for both
// spellings.
[require(hlsl)]
public struct HlslGen : IGen<int>
/*CHECK:
              ^^^^^^^ 'HlslGen' is missing 'metal'
*/
{
    uint get() { return 1; }
//CHECK: ^^^ 'get' is missing 'metal'
}

[require(hlsl)]
public struct HlslNG : ING
/*CHECK:
              ^^^^^^ 'HlslNG' is missing 'metal'
*/
{
    uint get() { return 1; }
//CHECK: ^^^ 'get' is missing 'metal'
}

#endif

[require(metal)]
public uint declaredMetal(MetalOnly<3> a)
{
    return a.value;
}

public uint undeclared(MetalOnly<3> a)
{
    return a.value;
}

[require(metal)]
public struct MetalGen : IGen<int>
{
    uint get() { return 1; }
}
