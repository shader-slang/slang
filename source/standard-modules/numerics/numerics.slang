/**
Experimental, representation-independent numeric interfaces and generic operations.

@remarks Importers must enable experimental features. The API is intentionally outside `core` so
its decomposition can evolve while practical experience is gathered from generic numeric code.
*/
[ExperimentalModule]
module numerics;

import slang.numerics.__builtin;

__include "interfaces.slang";
__include "builtin-conformances.slang";
__include "floating-point-conformances.slang";
__include "cooperative-vector-conformances.slang";
__include "wrappers.slang";
