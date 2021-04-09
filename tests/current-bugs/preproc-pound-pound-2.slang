//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

#define A a
#define B b
#define OBJ A ## B

// Should output AB 
// Slang outputs ab

OBJ

