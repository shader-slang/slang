//TEST_IGNORE_FILE:

// This file is used by multi-file-import.slang
// It imports module-b and uses interface I

import module_b;

struct S { }
extension S : I { }

[numthreads(1,1,1)]
[shader("compute")]
void computeMain() {}
