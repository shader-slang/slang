// The exit code of a `slang` invocation with an unknown dispatcher flag is 1 (an invalid command
// line), which exercises the DISPATCHER arm of the exit-code check with a non-default
// expectation: removing `exit-code=1` fails this lane with "expected exit code 0, but the exit
// code is 1".
//TEST:DISPATCHER(filecheck=CHECK,exit-code=1): --bogus-flag version

// CHECK: result code = 1
// CHECK: slang: unknown flag '--bogus-flag'
