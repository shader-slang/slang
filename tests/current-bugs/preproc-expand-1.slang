//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

#define OPEN (
#define CLOSE )

#define C(x) x

#define B(x) x

B(C OPEN Hi CLOSE)

