//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Two transparent variables that user code builds through `syntax` can have the same type.
// Lookup then finds the same member through both, and the member does not conflict with
// itself (shader-slang/slang#13876).

syntax my_transparent : TransparentModifier;

struct Shared
{
    int sharedMember;
}

my_transparent ConstantBuffer<Shared> first;
my_transparent ConstantBuffer<Shared> second;
