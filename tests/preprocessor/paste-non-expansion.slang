//DIAGNOSTIC_TEST:SIMPLE:-E

// NOTE! This test should *fail*, if preprocessor is working correctly!

// This demonstrates the existance of a bug in Slang preprocessor macro expansion. Could be due to incorrect paste handling
// or perhaps the rules around parameter expansion.

#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)

#define SOMETHING someThing

// Should be someThingElse
CONCAT(SOMETHING, Else)
// Should be SOMETHINGAnother, but Slang expands to produce someThingAnother
CONCAT2(SOMETHING, Another)