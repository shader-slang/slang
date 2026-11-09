//TEST:SIMPLE(exit-code=-1):
// #include support

int foo() { return 0; }

#include "include-a.slang.h"
#include "../preprocessor/include-a.slang.h"
#include "./include-a.slang.h"

int baz() { return bar(); }