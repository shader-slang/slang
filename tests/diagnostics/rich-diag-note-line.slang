// The first diagnostic is E30201 "function 'main' already has a body" for the second
// definition, with an attached note pointing at the first one; the note's location arrives
// via a ` --- ` marker line. The SIMPLE lane pins the raw headers of both records.
//TEST:SIMPLE_LINE(filecheck=NOTECHECK):-entry main -stage compute -target spirv-asm
//TEST:SIMPLE(filecheck=NOTECHK,exit-code=-1):-entry main -stage compute -target spirv-asm

[shader("compute")]
[numthreads(1,1,1)]
void main() { }

[shader("compute")]
[numthreads(1,1,1)]
void main() { }
// NOTECHECK: [[#@LINE-1]]
// NOTECHK: error[E30201]: function 'main' already has a body
// NOTECHK: note: see previous definition of 'main'
// NOTECHK: --- tests/diagnostics/rich-diag-note-line.slang:9:6
