// entry-point-redundant-stage.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-stage vertex -stage vertex

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-entry vsMain -stage vertex -stage vertex

// CHECK: warning[E00030]: the stage 'vertex' was specified more than once

float4 main() : SV_Position { return 0; }
float4 vsMain() : SV_Position { return 0; }
