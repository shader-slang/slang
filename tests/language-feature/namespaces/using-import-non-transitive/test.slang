//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
// non-exhaustive: assert only the primary diagnostic.
//
// shader-slang/slang#11443: `import` is not transitive for another module's
// `__include`d files. `tmid` does a plain `import tleaf;`, and tleaf's
// `__include`d file declares `tleafHelper`. Importing `tmid` must NOT re-export
// tleaf's `FileDecl` (whose `parentDecl` is tleaf, not tmid), so `tleafHelper` is
// not visible here. This pins the `containerDecl->parentDecl == moduleDecl` clause
// in the re-export filter: a bare `as<FileDecl>` check would wrongly re-export
// tleaf's foreign `FileDecl` through tmid and make `tleafHelper` visible.

module test;

import tmid;

int useTransitive()
{
    return tleafHelper(1);
//CHECK: undefined identifier 'tleafHelper'
}
