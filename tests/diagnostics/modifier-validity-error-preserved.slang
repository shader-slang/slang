// shader-slang/slang#12558: `inline` is meaningful only on a callable, so applying it to a
// non-callable must be an error (E31201). This exercises both verdict paths: an aggregate type
// hits the authored `Error` rule (AggTypeDecl, InlineModifier), and a typedef matches no rule and
// is rejected by the legacy `checkModifierAllowedOnDecl` fallback. Exhaustive `diag` mode binds
// each annotation to its `inline` token, so exactly one E31201 per bad declaration is verified
// (no spurious extra, and no softer E31232 in place of the hard error). Both decls are used
// afterward to confirm the declaration recovers cleanly once the bad modifier is dropped.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target spirv

inline struct NotACallable
/*diag:
^^^^^^ modifier not allowed
^^^^^^ modifier 'inline' is not allowed here.
*/
{
    int x;
}

inline typedef int NotACallableAlias;
/*diag:
^^^^^^ modifier not allowed
^^^^^^ modifier 'inline' is not allowed here.
*/

[numthreads(1, 1, 1)]
void computeMain()
{
    NotACallable v;
    v.x = 0;
    NotACallableAlias a = v.x;
}
