//DIAGNOSTIC_TEST:SIMPLE:-E

// First check we can define __LINE__ all C/C++ compilers test allowed this without error, behaviour
// did change based on compiler. Visual Studio ignored the define. 
// Other compilers (Clang,Gcc) honored it, they may give a warning. Once it has been defined, doing #undef actually *disables* builtin macro.

// Lets try some macro pasting. NOTE strictly speaking we need CONCAT2 (as tested on other compilers, but Slang does not indicating a preprocessor bug!)
#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)

CONCAT(Thing, __LINE__)

// Lets see if works okay in conditionals

#if __LINE__ == 15
Yeppers
#else
Nopers
#endif

// Lets try a paste in a conditional
#if CONCAT(__LINE__, __LINE__) == 2222
PasteConditional
#endif

// Lets check that we pick up the nominal line
#line 10
__LINE__

// Reset the line numbering
#line default

// __LINE__ number will be for the macro invocation, not the expansion

#define SOMEMACRO __LINE__

SOMEMACRO