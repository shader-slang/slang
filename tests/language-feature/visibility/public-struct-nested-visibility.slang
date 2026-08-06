// Nested-struct visibility under the 2026 public-by-default rule: an unmodified member of an
// aggregate inherits the aggregate's effective visibility, and a nested struct follows the same
// rule. So a public nested struct (and its unmodified members) is accessible across modules, while
// an `internal`/`private` nested struct is not.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language slang 2025

import "public-struct-nested-visibility-lib";

int usePublic(PublicOuter.PublicInner i)
{
    return i.nestedUnmodified; // OK: public nested struct and its unmodified member are public.
}

// Error: an `internal` nested struct is not accessible across the module boundary.
int useInternal(PublicOuter.InternalInner i)
//CHECK:                    ^^^^^^^^^^^^^ declaration not accessible
//CHECK:                    ^^^^^^^^^^^^^ 'InternalInner' is not accessible from the current context.
{
    return i.nestedUnmodified;
}

// Error: a `private` nested struct is not accessible across the module boundary either.
int usePrivate(PublicOuter.PrivateInner i)
//CHECK:                   ^^^^^^^^^^^^ declaration not accessible
//CHECK:                   ^^^^^^^^^^^^ 'PrivateInner' is not accessible from the current context.
{
    return i.nestedUnmodified;
}
