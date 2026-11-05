//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// User code can make a variable transparent through `syntax`. Lookup then finds the members of
// the variable's type, so they conflict with later declarations of the same name, as the
// members of a `cbuffer` do (shader-slang/slang#13876).

syntax my_transparent : TransparentModifier;
syntax my_group : ImplicitParameterGroupVariableModifier;

struct S
{
    int x;
/*CHECK:
        ^ see previous declaration of 'x'
*/
}

my_transparent my_group ConstantBuffer<S> cb;

static const int x = 1;
/*CHECK:
                 ^ warning E30204
                 ^ declaration of 'x' conflicts with existing declaration
*/
