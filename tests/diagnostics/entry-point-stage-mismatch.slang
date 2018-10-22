// entry-point-stage-mismatch.slang

// Confirm that we diagnose when stage specified via command
// line doesn't match what was specified via attribute.

//TEST:SIMPLE:-entry main -stage vertex

[shader("compute")]
void main()
{}
