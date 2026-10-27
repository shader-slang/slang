//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// User code can attach the internal buffer block modifiers to a variable through `syntax`.
// No element struct comes before such a variable, so the redeclaration check treats it as an
// ordinary declaration instead of failing an internal assertion (shader-slang/slang#13876).

syntax my_transparent : TransparentModifier;
syntax my_group : ImplicitParameterGroupVariableModifier;

struct S
{
    int x;
}

my_transparent my_group ConstantBuffer<S> cb;
