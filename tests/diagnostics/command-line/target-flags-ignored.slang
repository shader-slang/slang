// target-flags-ignored.slang

// Cases where a command-line compile reports a `-floating-point-mode` as ignored
// because it applies to no target. Of the target options, a compile checks only
// the floating-point mode in these cases.

// Case 1: no target. An API caller keeps the mode for the targets it adds,
// but a command-line compile has no target to apply it to.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-floating-point-mode precise
//CHECK1: target options were specified, but no target was specified with '-target'

// Case 2: a floating-point mode before the first `-target` when there are multiple targets.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-floating-point-mode precise -target dxbc -target spirv
//CHECK2: when using multiple targets, any target options must follow the '-target' they apply to
