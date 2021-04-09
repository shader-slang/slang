//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// So the real question here is what happens with pasting and stringifying. 
// As per the spec they have to be performed *before* expansion. If when we do an expansion
// we add to the stream FunctionArg that holds the 'to be' lazily expanded parameter we have a problem
//
// If we move to a new stream *and* the token afterwards, is ## 


#define CONCAT(a, b) a ## b

#define A a
#define B b

#define A2 A
#define B2 B

#define STRINGIFY(x) #x

// Should be 
// CONCAT(a, b) A2B2 CONCAT(a, b)
// CONCAT is disabled, A2 and B2 are processed on next pass
// A2 B2 are first and last tokens pre expansion args
// 
// Slang outputs 
// ab ab ab 

CONCAT(CONCAT(A, B) A2, B2 CONCAT(A, B))
