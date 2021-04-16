//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// Should produce: Hi
// Slang produces: C ( Hi ) 

#define OPEN (
#define CLOSE )

#define C(x) x

#define B(x) x

B(C OPEN Hi CLOSE)

