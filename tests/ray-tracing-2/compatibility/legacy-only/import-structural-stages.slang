//TEST:SIMPLE: -experimental-feature -I tests/ray-tracing-2/frontend/module -entry legacyMiss -stage miss -no-codegen

import refined_miss_library;

// The imported module defines structural stages. Its declarations must not prevent selecting
// this native entry, whose signature has no structural stage-input view.
void legacyMiss() {}
