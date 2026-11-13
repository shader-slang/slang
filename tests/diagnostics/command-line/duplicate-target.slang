// duplicate-target.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=1):-target hlsl -target spirv -target hlsl
//CHECK: the target 'hlsl' has been specified more than once
