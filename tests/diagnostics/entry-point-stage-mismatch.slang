// entry-point-stage-mismatch.slang

// Confirm that we diagnose when stage specified via command
// line doesn't match what was specified via attribute.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-entry main -stage vertex

[shader("compute")]
void main()
/*CHECK:
     ^^^^ entry point 'main' being compiled for the 'vertex' stage
*/
{}
