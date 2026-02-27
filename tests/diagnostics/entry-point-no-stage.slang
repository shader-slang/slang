// entry-point-no-stage.slang

// Confirm that we generate a diagnostic when
// compiling an entry point without any
// stage specified.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-entry main

void main()
/*CHECK:
     ^^^^ no stage specified for entry point
     ^^^^ no stage specified for entry point 'main'; use either a '[shader("name")]' function attribute or the '-stage <name>' command-line option to specify a stage
*/
{}
