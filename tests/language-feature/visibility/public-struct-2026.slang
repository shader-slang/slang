// A `public` struct/class defined in a Slang 2026 library exposes its unmodified members as
// `public`; explicit `internal`/`private` members still stay hidden across the module boundary.
// The consumer below is pinned to an older language version to show that the elevation is driven
// by the *defining* (provider) module's version, not the importing module's.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language slang 2025

import "public-struct-2026-lib";

void test(PublicStruct s, PublicClass c, PublicOuter.Inner inner)
{
    int a = s.unmodified;         // OK: unmodified member of a public struct is public in 2026.
    int b = s.explicitPublic;     // OK: explicitly public.
    int d = c.unmodified;         // OK: same rule applies to `class`.
    int h = inner.nestedUnmodified; // OK: transitively public through the public outer struct.
    PublicOuter.Color col = PublicOuter.Color.Red; // OK: nested enum + case public in 2026.

    // Error: explicit `internal` nested enum override is not visible across modules.
    PublicOuter.Hidden hid = PublicOuter.Hidden.A;
/*CHECK:
                ^^^^^^ declaration not accessible
                ^^^^^^ 'Hidden' is not accessible from the current context.
                                         ^^^^^^ declaration not accessible
                                         ^^^^^^ 'Hidden' is not accessible from the current context.
*/
    // Error: an `internal` nested struct is not effectively public, so it (and its members) stay
    // internal in 2026 — the feature elevates only through effectively-public parents.
    PublicOuter.InternalInner ii;
/*CHECK:
                ^^^^^^^^^^^^^ declaration not accessible
                ^^^^^^^^^^^^^ 'InternalInner' is not accessible from the current context.
*/
    // Error: explicit `internal` override is not visible across modules.
    int e = s.explicitInternal;
/*CHECK:
              ^^^^^^^^^^^^^^^^ declaration not accessible
              ^^^^^^^^^^^^^^^^ 'explicitInternal' is not accessible from the current context.
*/
    // Error: explicit `private` override is not visible across modules.
    int f = s.explicitPrivate;
/*CHECK:
              ^^^^^^^^^^^^^^^ declaration not accessible
              ^^^^^^^^^^^^^^^ 'explicitPrivate' is not accessible from the current context.
*/
}
