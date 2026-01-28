// diagnostic-annotation-util.h
#ifndef SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H
#define SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H

#include "../../source/core/slang-basic.h"

namespace Slang
{

/// Utility for checking diagnostic test annotations.
///
/// Diagnostic annotations support two formats:
///
/// 1. Simple substring matching:
///   //PREFIX expected substring
///
/// 2. Position-based matching with column indicators:
///   some code line
///   //PREFIX        ^^^^^ Expected message substring
///
/// Where PREFIX is specified via the test command option: DIAGNOSTIC_TEST(diag=PREFIX):...
///
/// Example:
///   //DIAGNOSTIC_TEST(diag=CHECK):SIMPLE:
///   void test() {
///       if (x);
///   //CHECK        ^ warning
///   }
///
/// The checker will verify diagnostics match the expected positions and messages.
struct DiagnosticAnnotationUtil
{
    /// Check diagnostic annotations in source file against machine-readable diagnostic output
    /// @param sourceText The source file contents containing annotations
    /// @param prefix The annotation prefix (e.g., "CHECK", "foo")
    /// @param machineReadableOutput The machine-readable diagnostic output from compiler
    /// @param outErrorMessage Error message if check fails
    /// @return true if all annotations matched, false otherwise
    static bool checkDiagnosticAnnotations(
        const UnownedStringSlice& sourceText,
        const UnownedStringSlice& prefix,
        const UnownedStringSlice& machineReadableOutput,
        String& outErrorMessage);
};

} // namespace Slang

#endif // SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H
