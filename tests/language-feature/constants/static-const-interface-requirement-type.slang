//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Anchor the front-end boundary that static const interface requirements only
// accept integer and bool types. Typeflow treats unsupported result types as a
// compiler invariant, so this should remain a user-facing diagnostic.

interface IInvalidStaticConstRequirement
{
    static const float scale;
}

// CHECK: error[E30306]: invalid static const requirement type
// CHECK: 'static const' requirement can only have int or bool type.
