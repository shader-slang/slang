// Regression guard: in a Slang 2025 module the public-by-default rule is gated off, so an
// unmodified member of a `public` struct stays internal and is not visible across modules.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language slang 2025 // pin the consumer module version explicitly.

import "public-struct-2025-lib";

void test(PublicStruct s)
{
    int b = s.explicitPublic;  // OK: explicitly public regardless of language version.

    // Error: unmodified member stays internal in 2025 (feature is 2026+).
    int a = s.unmodified;
/*CHECK:
              ^^^^^^^^^^ declaration not accessible
              ^^^^^^^^^^ 'unmodified' is not accessible from the current context.
*/
}
