// diagnostic-annotation-util.h
#ifndef SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H
#define SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H

#include "../../source/core/slang-basic.h"

namespace Slang
{

/// Utility for checking diagnostic test annotations.
///
/// Diagnostic annotations allow test files to specify expected substrings
/// in the program output using a simple annotation format:
///
///   //PREFIX expected substring
///
/// Where PREFIX is specified via the test command option: DIAGNOSTIC_TEST(diag=PREFIX):...
///
/// Example:
///   //DIAGNOSTIC_TEST(diag=CHECK):SIMPLE:
///   void test() {
///       if (x);  // This triggers a warning
///   }
///   //CHECK warning
///   //CHECK empty statement
///
/// The checker will verify that all annotated substrings appear somewhere in the output.
struct DiagnosticAnnotationUtil
{
    /// Parse diagnostic annotations from source file
    /// @param sourceText The source file contents
    /// @param prefix The annotation prefix (e.g., "CHECK", "foo")
    /// @param outAnnotations List to populate with expected substrings
    /// @return SLANG_OK on success, SLANG_FAIL if parsing fails
    static SlangResult parseAnnotations(
        const UnownedStringSlice& sourceText,
        const UnownedStringSlice& prefix,
        List<String>& outAnnotations);

    /// Check that all annotations appear in the output
    /// @param annotations List of expected substrings from parseAnnotations
    /// @param output The actual program output to check
    /// @param outMissingAnnotations List to populate with annotations not found in output
    /// @return true if all annotations found, false otherwise
    static bool checkAnnotations(
        const List<String>& annotations,
        const UnownedStringSlice& output,
        List<String>& outMissingAnnotations);
};

} // namespace Slang

#endif // SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H
