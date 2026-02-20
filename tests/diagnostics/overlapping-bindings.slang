// overlapping-bindings.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

// Two parameters with the same `register:

Texture2D a : register(t0);
//CHECK:  ^ see declaration of 'a'

Texture2D b : register(t0);
/*CHECK:
          ^ explicit binding overlap
          ^ explicit binding for parameter 'b' overlaps with parameter 'a'
*/

// Parameters marked to ignore overlap:

[allow("overlapping-bindings")]
Texture2D c : register(t1);

[allow("overlapping-bindings")]
Texture2D d : register(t1);
