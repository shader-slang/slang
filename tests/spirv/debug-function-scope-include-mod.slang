// The module that performs the __include for debug-function-scope-include.slang. No //TEST:
// directive — it is imported by that file.
//
// Keep the __include here rather than in the file holding the entry point. The module-global scope a
// DebugFunction falls back to is pinned to the entry point's compilation unit, so if this __include
// moved into the entry-point file the broken fallback and the correct answer would be the same inst,
// and the test would pass even when the resolution is broken.

module debug_function_scope_include_mod;

__include "debug-function-scope-included";
