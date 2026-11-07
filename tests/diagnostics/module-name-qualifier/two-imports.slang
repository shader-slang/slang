// Each of two imported module names used as a qualifier gets the module-name
// note (shader-slang/slang#14063).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import mod_a;
import mod_b;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = ModA.twice(o[0]);
//CHECK:   ^^^^ error E30015
//CHECK:   ^^^^ undefined identifier 'ModA'.
//CHECK:   ^^^^ 'ModA' names a module; module names cannot be used as qualifiers, types, or values
    o[1] = ModB.twice(o[1]);
//CHECK:   ^^^^ error E30015
//CHECK:   ^^^^ undefined identifier 'ModB'.
//CHECK:   ^^^^ 'ModB' names a module; module names cannot be used as qualifiers, types, or values
}
