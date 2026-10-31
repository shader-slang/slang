// target-flags-ignored.slang

// Cases where a target option such as `-floating-point-mode` gets ignored
// because it doesn't apply to any target

// Case 1: no target. An API caller keeps the option for the targets it adds,
// but a command-line compile has no target to apply it to.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-floating-point-mode precise
//CHECK1: target options were specified, but no target was specified with '-target'

// Case 2: a target option before any `-target` when there are multiple targets.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-floating-point-mode precise -target dxbc -target spirv
//CHECK2: when using multiple targets, any target options must follow the '-target' they apply to
