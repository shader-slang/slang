#language slang 2025

module "public-struct-2025-lib";

// The same `public struct` shape as the 2026 test, but in a 2025 module. The public-by-default
// rule is gated on language version >= 2026, so here unmodified members stay internal.
public struct PublicStruct
{
    int unmodified;            // no modifier -> internal in 2025.
    public int explicitPublic; // explicitly public -> visible across modules.
}
