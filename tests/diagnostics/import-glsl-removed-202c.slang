// `import glsl` is a legacy Slang compatibility path, not a way to change the source language.
// Slang 202c removes that path while earlier language versions retain it for source compatibility.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language slang 202c

import glsl;
/*CHECK:
       ^^^^ importing the builtin `glsl` module is not allowed in Slang 202c or later
       ^^^^ select GLSL as the source language instead of importing its builtin module into Slang source
*/
