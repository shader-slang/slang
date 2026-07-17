// This is a non-test helper module for debug-source-import-include.slang (no //TEST: directive).
// It is `import`ed by the entry-point file and itself `#include`s a sidecar, so the three physical
// files (entry point, this module, the sidecar) must each get exactly one DebugSource record.

#include "debug-source-import-include-sidecar.slang"
