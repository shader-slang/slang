//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

interface IGenericRequirement
{
    void usePack<each T>();
//CHECK: ^^^^^^^ see interface requirement declaration of 'usePack'
}

struct StricterImpl : IGenericRequirement
{
    // A stricter implementation generic signature must not satisfy a looser
    // interface requirement.
    void usePack<each T>() where nonempty(T) {}
//CHECK: ^^^^^^^ interface requirement mismatch
//CHECK: ^^^^^^^ member 'usePack' does not match interface requirement.
}
