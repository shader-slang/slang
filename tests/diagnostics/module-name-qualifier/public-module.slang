// The name of an imported `public module` is not a qualifier
// (shader-slang/slang#14063).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import mod_public;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = ModP.twice(o[0]);
//CHECK:   ^^^^ error E30015
//CHECK:   ^^^^ undefined identifier 'ModP'.
//CHECK:   ^^^^ 'ModP' names a module; module names cannot be used as qualifiers, types, or values
}
