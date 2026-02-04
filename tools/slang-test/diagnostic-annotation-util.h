// diagnostic-annotation-util.h
//
// Diagnostic Annotation System for slang-test
// ============================================
//
// This system allows test files to specify expected compiler diagnostics (warnings, errors)
// using inline annotations. The test runner automatically verifies that diagnostics appear
// at the expected locations with the expected messages.
//
// QUICK START
// -----------
//
// 1. Enable diagnostic annotations in your test:
//    //DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
//
// 2. Add annotations to indicate expected diagnostics:
//    void test() {
//        if (x);
//    //CHECK   ^ empty statement
//    }
//
// When the test runs, slang-test will:
// - Compile with -enable-machine-readable-diagnostics
// - Parse the machine-readable diagnostic output
// - Verify each annotation matches a diagnostic in the output
//
// ANNOTATIONS
// ------------------
//
// Use line comments to annotate the preceding (non-annotation) line
// The carets (^) should align directly with the source column:
//
//     int x = undefined;
//     //CHECK ^^^^^^^^^ undeclared identifier
//
// Syntax:
//     //PREFIX   [spaces]   ^^^   message substring
//            └─ spaces position the caret to match the source column
//                            └─ one or more carets indicate the column range
//                                  └─ expected message substring
//
// The caret positions in the annotation line directly corresponds to the columns
// in the source. For example, if a caret is at character position 15
// in the annotation line, it expects a diagnostic at column 15 in the source.
//
// Use block comments when you need to place carets at early columns that
// would be taken up by "//CHECK", or when you have multiple diagnostics on the
// same line:
//
//     if (x == y);
//     /*CHECK
//     ^ don't use if here
//               ^ this semicolon isn't great
//     */
//
// Each line in the block comment is treated as a separate annotation. Empty
// lines are ignored. The caret position directly indicates the column in the
// preceding source line.
//
// POSITION-BASED vs SUBSTRING MATCHING
// -------------------------------------
//
// Position-Based (contains ^):
//     Checks that a diagnostic appears at a specific line and column with
//     a message containing the expected substring.
//
//     if (abc(foo));
//     //CHECK      ^ empty statement
//
//     This expects:
//     - Line 1 (the preceding non-annotation line)
//     - Column 9 (where the ^ is positioned)
//     - Message contains "empty statement"
//
// Simple Substring (no ^):
//     Checks that a diagnostic message containing the substring appears
//     somewhere in the output, regardless of location.
//
//     //CHECK unused variable
//
//     This expects:
//     - Any line, any column
//     - Message contains "unused variable"
//
// MATCHING AGAINST SEVERITY AND ERROR CODES
// ------------------------------------------
//
// Annotations can match against multiple fields of a diagnostic:
// - The message text
// - The severity (e.g., "warning", "error")
// - The error code (e.g., "E20101")
// - Combined "severity errorCode" (e.g., "warning E20101")
//
// Examples:
//     if (x);
//     //CHECK   ^ warning
//     //CHECK   ^ E20101
//     //CHECK   ^ warning E20101
//     //CHECK   ^ empty statement
//
// This is useful for checking specific error codes without needing the full message.
//
// EXHAUSTIVE vs NON-EXHAUSTIVE MODE
// ----------------------------------
//
// By default, tests are EXHAUSTIVE: all diagnostics must have annotations.
// If the compiler produces diagnostics that aren't annotated, the test fails.
//
// Example (fails in exhaustive mode):
//     //DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
//     void test() {
//         if (1);
//     //CHECK   ^ empty statement
//         // Missing annotation for "implicit conversion" warning!
//     }
//
// Use non-exhaustive mode to only check that annotations match:
//     //DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
//     void test() {
//         if (1);
//     //CHECK   ^ empty statement
//         // implicit conversion warning is OK to ignore
//     }
//
// When to use non-exhaustive:
// - Testing a specific diagnostic while ignoring others
// - Incremental test development (annotate one diagnostic at a time)
// - When other diagnostics are noise for the test
//
// Prefer exhaustive mode (default) to ensure no unexpected diagnostics appear.
//
// COLUMN RANGE MATCHING
// ---------------------
//
// Use multiple carets to specify the exact column range:
//
//     MyStruct xyz = {};
//     //CHECK  ^^^ unused variable
//
// Single caret (^):    Expects diagnostic at exactly that column (e.g., 5-5)
// Multiple carets (^^^): Expects diagnostic spanning that range (e.g., 5-7)
//
// The column range is inclusive: "^^^" at position 5 means columns 5-7.
//
// MACHINE-READABLE DIAGNOSTIC FORMAT
// -----------------------------------
//
// When diag=PREFIX is specified, slang-test automatically adds
// -enable-machine-readable-diagnostics to the compiler command line.
//
// This produces tab-separated diagnostic output:
//     E<code>\t<severity>\t<filename>\t<beginline>\t<begincol>\t<endline>\t<endcol>\t<message>
//
// Example:
//     E20101	warning	test.slang	9	13	9	13	potentially unintended empty
//     statement at this location; use {} instead.
//
// The checker parses this format and matches against your annotations.
//
// ERROR REPORTING
// ---------------
//
// When a check fails, detailed error messages show what was expected vs actual:
//
// Example 1 - Wrong column:
//     Position-based match failed:
//       Expected: Line 9, column 13, message containing: "empty statement"
//       Actual diagnostics on line 9:
//         Line 9, column 11: "potentially unintended empty statement..."
//         Line 9, column 9: "implicit conversion from 'int' to 'bool'..."
//       Note: Column position(s) don't match expected column 13
//
// Example 2 - Wrong message:
//     Position-based match failed:
//       Expected: Line 9, column 11, message containing: "wrong text"
//       Actual diagnostics on line 9:
//         Line 9, column 11: "potentially unintended empty statement..."
//       Note: Column position matched but message didn't contain expected substring
//
// Example 3 - No diagnostics on line:
//     Position-based match failed:
//       Expected: Line 15, column 5, message containing: "error"
//       No diagnostics found on line 15
//
// Example 4 - Substring not found anywhere:
//     Simple substring match failed:
//       Expected substring: "unused variable"
//       Actual diagnostics:
//         Line 5, column 10: "implicit conversion..."
//         Line 8, column 12: "empty statement..."
//

#ifndef SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H
#define SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H

#include "../../source/core/slang-basic.h"

namespace Slang
{

/// Utility for checking diagnostic test annotations.
///
/// See the extensive documentation above for usage details.
struct DiagnosticAnnotationUtil
{
    /// Check diagnostic annotations in source file against machine-readable diagnostic output
    ///
    /// @param sourceText The source file contents containing annotations
    /// @param prefix The annotation prefix (e.g., "CHECK", "foo")
    /// @param machineReadableOutput The machine-readable diagnostic output from compiler
    ///                              (tab-separated format from
    ///                              -enable-machine-readable-diagnostics)
    /// @param exhaustive If true (default), fail if any diagnostics lack annotations
    ///                   If false, only check that annotations match (ignore extra diagnostics)
    /// @param outErrorMessage Detailed error message if check fails, including expected vs actual
    /// @return true if all checks passed, false otherwise
    static bool checkDiagnosticAnnotations(
        const UnownedStringSlice& sourceText,
        const UnownedStringSlice& prefix,
        const UnownedStringSlice& machineReadableOutput,
        bool exhaustive,
        String& outErrorMessage);
};

} // namespace Slang

#endif // SLANG_TEST_DIAGNOSTIC_ANNOTATION_UTIL_H
