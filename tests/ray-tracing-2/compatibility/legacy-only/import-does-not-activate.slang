//TEST:SIMPLE: -experimental-feature -entry legacyMiss -stage miss -no-codegen

import slang.raytracing;

[shader("miss")]
void legacyMiss() {}
