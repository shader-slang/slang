// global-uniform.slang
//TEST:SIMPLE:-target hlsl
//TEST:COMMAND_LINE_SIMPLE:-target hlsl

// Any attempt to declare a global variable that actually declares a
// global uniform should be diagnosed as unsupported.

uniform float a;

const uint4 b = uint4(0,1,2,3);

struct C { float x; int y; };
C c;
