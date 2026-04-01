// profile-ignored.slang

// Cases where a `-profile` option gets ignored
// because it doesn't apply to any target

// Case 1: multiple (conflicting) profiles, so we can't infer a single stage
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-profile sm_5_0 -profile glsl_450
//CHECK1: a '-profile' option was specified, but no target was specified with '-target'


// Case 2: a `-profile` option before any `-target`, possibly because
// the user is specifying things in the wrong order.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-profile sm_5_0 -target dxbc -profile glsl_450 -target spirv
//CHECK2: when using multiple targets, any '-profile' option must follow the '-target' it applies to
