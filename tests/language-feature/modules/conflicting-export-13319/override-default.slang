//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: overrides the extern
// default `kDefault`, which is not a conflict.

import contract;

export static const uint kDefault = 7;
