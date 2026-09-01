#language slang 2026

module "public-struct-2026-lib";

// A `public` struct in a 2026 module: unmodified members become `public` by default,
// explicit `internal`/`private` still override.
public struct PublicStruct
{
    int unmodified;                // no modifier -> public in 2026.
    public int explicitPublic;
    internal int explicitInternal; // override -> stays internal.
    private int explicitPrivate;   // override -> stays private.
}

// The same rule applies to `class`.
public class PublicClass
{
    int unmodified;                // no modifier -> public in 2026.
}

// Transitivity: because the rule keys off the parent's *effective* visibility (not the literal
// keyword), an unmodified nested struct inside a public struct is itself public, and so are its
// unmodified members. This is the behavior that distinguishes keying on effective visibility from
// a syntactic `public`-keyword check.
public struct PublicOuter
{
    struct Inner
    {
        int nestedUnmodified;      // public: Inner is public (public parent), so its member is too.
    }

    // An `internal` nested struct is not effectively public, so its unmodified members stay
    // internal even in 2026 — the rule elevates only through effectively-public parents.
    internal struct InternalInner
    {
        int stillInternal;
    }

    enum Color { Red, Green, Blue } // public: nested enum of a public struct (cases inherit).
    internal enum Hidden { A, B }   // override -> stays internal.
}
