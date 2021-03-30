//DIAGNOSTIC_TEST:SIMPLE:-E

// First check we can define __LINE__ all C/C++ compilers test allowed this without error, behaviour
// did change based on compiler. Visual Studio ignored the define, other compilers honored it.
// Slang is going to honor it.

#define __LINE__ "Hello"

__LINE__

#undef __LINE__

__LINE__

#define __LINE__(x) "Is" x

__LINE__("Anybody")
__LINE__(There)

#undef __LINE__

// Lets try some macro pasting

#define CONCAT(x, y) x ## y

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