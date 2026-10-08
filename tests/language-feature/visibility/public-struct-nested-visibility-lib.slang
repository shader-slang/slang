#language slang 2026

module "public-struct-nested-visibility-lib";

// In a 2026 module, an unmodified member of an aggregate inherits the aggregate's effective
// visibility. A nested struct follows the same rule, so the three nested structs below take the
// visibility of their modifier (or the public parent, for the unmodified one).
public struct PublicOuter
{
    struct PublicInner // unmodified -> public (public parent)
    {
        int nestedUnmodified; // -> public
    }

    internal struct InternalInner // explicit internal
    {
        int nestedUnmodified; // -> internal (inherits the internal parent)
    }

    private struct PrivateInner // explicit private
    {
        int nestedUnmodified; // -> private (inherits the private parent)
    }
}
