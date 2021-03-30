//DIAGNOSTIC_TEST:SIMPLE:-E

// First check we can define __LINE__ all C/C++ compilers test allowed this without error, behaviour
// did change based on compiler. Visual Studio ignored the define. 
// Other compilers (Clang,Gcc) honored it, they may give a warning. Once it has been defined, doing #undef actually *disables* builtin macro.

#define __FILE__(x) "Is" x

__FILE__("Anybody")
__FILE__(There)

#undef __FILE__

__FILE__