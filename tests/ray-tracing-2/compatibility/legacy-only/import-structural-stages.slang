//TEST:SIMPLE: -experimental-feature -I tests/ray-tracing-2/frontend/module -entry legacyMiss -stage miss -no-codegen

import refined_miss_library;

// The imported module defines structural stages, but this module only selects a native stage.
// The two modules must not share the bookkeeping that rejects mixing both APIs in one module.
void legacyMiss() {}
