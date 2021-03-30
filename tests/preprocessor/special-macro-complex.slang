//DIAGNOSTIC_TEST:SIMPLE:-E

// First check we can define __LINE__ all C/C++ compilers test allowed this without error, behaviour
// did change based on compiler. Visual Studio ignored the define, other compilers honored it, but may give a warning (Clang).
// Slang is going to honor it.

#define __LINE__ "Hello"

__LINE__

#undef __LINE__

__LINE__

#define __LINE__(x) "Is" x

__LINE__("Anybody")
__LINE__(There)

#undef __LINE__

// Lets try some macro pasting. NOTE strictly speaking we need CONCAT2 (as tested on other compilers, but Slang does not indicating a preprocessor bug!)
#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)

CONCAT(Thing, __LINE__)

// Lets see if works okay in conditionals

#if __LINE__ == 30
Yeppers
#else
Nopers
#endif

// Lets try a paste in a conditional
#if CONCAT(__LINE__, __LINE__) == 3737
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