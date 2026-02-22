//

// The file is meant to be included multiple times, to produce different
// pieces of declaration/definition code related to diagnostic messages
//
// Each diagnostic is declared here with:
//
//     DIAGNOSTIC(id, severity, name, messageFormat)
//
// Where `id` is the unique diagnostic ID, `severity` is the default
// severity (from the `Severity` enum), `name` is a name used to refer
// to this diagnostic from code, and `messageFormat` is the default
// (non-localized) message for the diagnostic, with placeholders
// for any arguments.

#ifndef DIAGNOSTIC
#error Need to #define DIAGNOSTIC(...) before including "DiagnosticDefs.h"
#define DIAGNOSTIC(id, severity, name, messageFormat) /* */
#endif

//
// All diagnostics have been migrated to slang-diagnostics.lua and its submodules.
// This file now only contains the DIAGNOSTIC macro setup and can be deprecated
// once all consumers are updated.
//
// Diagnostic modules:
//   slang-diagnostics-preprocessing.lua - 15xxx preprocessing diagnostics
//   slang-diagnostics-parsing.lua - 2xxxx parsing diagnostics
//   slang-diagnostics-semantic-checking-1.lua - 3xxxx semantic checking (core)
//   slang-diagnostics-semantic-checking-2.lua - 3xxxx semantic checking (types/overloads)
//   slang-diagnostics-semantic-checking-3.lua - Include/Visibility/Capability
//   slang-diagnostics-semantic-checking-4.lua - Attributes
//   slang-diagnostics-semantic-checking-5.lua - COM/Derivative/Extern
//   slang-diagnostics-semantic-checking-6.lua - Differentiation/Modifiers/GLSL/HLSL
//   slang-diagnostics-semantic-checking-7.lua - Link Time/Generics/Inheritance
//   slang-diagnostics-semantic-checking-8.lua - Properties/Accessors/BitFields
//   slang-diagnostics-semantic-checking-9.lua - Operators/Literals/Entry Points
//   slang-diagnostics-semantic-checking-10.lua - Matrices/Compute stages
//   slang-diagnostics-semantic-checking-11.lua - Loops/Interfaces/Mesh
//   slang-diagnostics-semantic-checking-12.lua - IL code generation (4xxxx)
//   slang-diagnostics-semantic-checking-13.lua - Resource validation (41xxx)
//   slang-diagnostics-semantic-checking-14.lua - Target code generation (5xxxx)
//   slang-diagnostics-semantic-checking-15.lua - Metal/SPIRV/GLSL/Autodiff/NVAPI
//   slang-diagnostics-semantic-checking-16.lua - Internal errors/Ray tracing/Coop matrix
//   slang-diagnostics-semantic-checking-17.lua - Standalone notes for cross-referencing
//

#undef DIAGNOSTIC
