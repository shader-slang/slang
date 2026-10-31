// An `import` that fails to load does not make its name a module name, so a
// use of that name gets no module-name note (shader-slang/slang#14063).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import mod_missing;
/*CHECK:
       ^^^^^^^^^^^ cannot open file 'mod_missing.slang'
*/

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = mod_missing.twice(o[0]);
//CHECK:   ^^^^^^^^^^^ error E30015
//CHECK:   ^^^^^^^^^^^ undefined identifier 'mod_missing'.
}
