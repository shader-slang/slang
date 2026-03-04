// entry-point-stage-mismatch.slang

// Confirm that we diagnose when stage specified via command
// line doesn't match what was specified via attribute.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-entry main -stage vertex

[shader("compute")]
void main()
/*CHECK:
     ^^^^ entry point stage mismatch
     ^^^^ entry point 'main' being compiled for the 'vertex' stage has a '[shader(...)]' attribute that specifies the 'compute' stage
*/
{}
