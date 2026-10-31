// A module is named by its declared name (`module ModD;`) and by the last
// segment of the path that imports it (`mod_d`, `mod_e`, `StrMod` without its
// extension); each of them used as a qualifier gets the module-name note
// (shader-slang/slang#14063).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import lib.mod_d;
import lib.mod_e;
import "lib/StrMod.slang";

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = ModD.twice(o[0]);
//CHECK:   ^^^^ error E30015
//CHECK:   ^^^^ undefined identifier 'ModD'.
//CHECK:   ^^^^ 'ModD' names a module; module names cannot be used as qualifiers, types, or values
    o[1] = mod_e.twice(o[1]);
//CHECK:   ^^^^^ error E30015
//CHECK:   ^^^^^ undefined identifier 'mod_e'.
//CHECK:   ^^^^^ 'mod_e' names a module; module names cannot be used as qualifiers, types, or values
    o[2] = mod_d.twice(o[2]);
//CHECK:   ^^^^^ error E30015
//CHECK:   ^^^^^ undefined identifier 'mod_d'.
//CHECK:   ^^^^^ 'mod_d' names a module; module names cannot be used as qualifiers, types, or values
    o[3] = StrMod.twice(o[3]);
//CHECK:   ^^^^^^ error E30015
//CHECK:   ^^^^^^ undefined identifier 'StrMod'.
//CHECK:   ^^^^^^ 'StrMod' names a module; module names cannot be used as qualifiers, types, or values
}
