//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

// Verify exhaustive diagnostic matching does not require duplicate
// annotations for the primary `span` row, while `note-span` rows still
// need an explicit position annotation.

// CHECK: warning[E39001]
// CHECK: note:
Texture2D a : register(t0);
//CHECK:  ^ see declaration of 'a'

Texture2D b : register(t0);
/*CHECK:
          ^ explicit binding overlap
          ^ explicit binding for parameter 'b' overlaps with parameter 'a'
*/
