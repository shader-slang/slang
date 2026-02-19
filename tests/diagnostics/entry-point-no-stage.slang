// entry-point-no-stage.slang

// Confirm that we generate a diagnostic when
// compiling an entry point without any
// stage specified.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-entry main

void main()
/*CHECK:
     ^^^^ no stage specified for entry point
*/
{}
