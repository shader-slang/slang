// pass-through-no-stage.hlsl

// Trying to compile in `-pass-through` mode without
// specifying a stage is an error, because the downstream
// compilers don't support inferring the stage from
// an attribute.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-pass-through fxc -entry main
//CHECK: no stage was specified for entry point 'main'; when using the '-pass-through' option, stages must be fully specified on the command line
