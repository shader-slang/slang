//TEST:SIMPLE: -experimental-feature -entry legacyMiss -stage miss -no-codegen

// Importing the module makes its declarations available without selecting the structural API.
// Importing the structural library leaves this ordinary native entry signature valid.
import slang.raytracing;

[shader("miss")]
void legacyMiss() {}
