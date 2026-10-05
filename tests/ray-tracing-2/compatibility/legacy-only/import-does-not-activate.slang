//TEST:SIMPLE: -experimental-feature -entry legacyMiss -stage miss -no-codegen

// Importing the module makes its declarations available without selecting the structural API.
// This module only defines a legacy miss entry point, so the import must not cause a mixed-API error.
import slang.raytracing;

[shader("miss")]
void legacyMiss() {}
