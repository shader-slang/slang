//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

import slang.raytracing;
/*CHECK:
       ^^^^^ 'slang/raytracing' is an experimental module, need to enable '-experimental-feature' to load this module
*/
