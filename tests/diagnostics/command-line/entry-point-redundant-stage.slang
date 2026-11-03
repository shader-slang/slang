// entry-point-redundant-stage.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-stage vertex -stage vertex

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-entry vsMain -stage vertex -stage vertex

// CHECK: was specified more than once
