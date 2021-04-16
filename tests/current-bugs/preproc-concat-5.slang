//DISABLE_DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// It should produce 'THING', as the original invocation should have disabled THING, 
// but it actually ends up in an infinite loop.

#define CONCAT(x, y) x ## y

#define THING2 THING
#define THING CONCAT(THING, 2)

THING
