//TEST:SIMPLE: -Itests/preprocessor/include
//TEST:SIMPLE: -DUSE_ANGLE_BRACKETS
//TEST:SIMPLE: -DUSE_ANGLE_BRACKETS -Itests/preprocessor/include

// #include support

int foo() { return 0; }

#ifdef USE_ANGLE_BRACKETS
#include <pragma-once-c.h>
#else
#include "pragma-once-c.h"
#endif

// If include worked this will be defined
#ifndef ONLY_DEFINED_ONCE_C
// And so hitting this indicates and error (and will fail as bar isn't defined)
int baz() { return bar(); }
#endif

