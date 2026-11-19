// `using ModA;` on an imported module name gives one error with the
// module-name note, and no second "expected a namespace" error
// (shader-slang/slang#14063). A non-namespace argument still gets that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import mod_a;
using ModA;
/*CHECK:
      ^^^^ error E30015
      ^^^^ undefined identifier 'ModA'.
      ^^^^ 'ModA' names a module; module names cannot be used as qualifiers, types, or values
*/

// A `using` of a declaration that is not a namespace is still diagnosed.
struct NotANamespace {}
using NotANamespace;
/*CHECK:
      ^^^^^^^^^^^^^ error E30061
      ^^^^^^^^^^^^^ expected a namespace, got a 'typeof(NotANamespace)'
*/

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = twice(o[0]);
}
