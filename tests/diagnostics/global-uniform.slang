// global-uniform.slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl
//DIAGNOSTIC_TEST:COMMAND_LINE_SIMPLE(diag=CHECK):-target hlsl

// An attempt to declare a global variable that actually declares a
// global shader parameter should be diagnosed, unless `uniform` was used.
// CHECK: warning[E39019]

uniform float a;

static const uint4 b = uint4(0,1,2,3);

struct C { float x; int y; };
C c;
/*CHECK:
  ^ implicit global shader parameter
  ^ 'c' is implicitly a global shader parameter
*/
