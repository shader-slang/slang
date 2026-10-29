//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing.portable;

// Importing the public API must not expose its compiler role enum.
typealias ExposedRole = rt::__RayTracingBuiltin;
//CHECK:                    ^^^^^^^^^^^^^^^^^^^ declaration not accessible
//CHECK:                    ^^^^^^^^^^^^^^^^^^^ '__RayTracingBuiltin' is not accessible from the current context.
