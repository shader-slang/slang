//TEST:SIMPLE:

// After an include, the preprocessor must resume the location map in this file.
// Expanding the first token before recording that position would instead use
// the macro definition's location in the included header.
#include "include-macro-location.slang.h"

DECLARATION value = 1;
