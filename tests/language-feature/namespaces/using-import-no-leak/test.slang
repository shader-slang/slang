//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
// non-exhaustive: assert only the primary leak diagnostic; an undefined identifier
// can also produce cascading diagnostics we don't want to pin here.

module test;

import helper;

// shader-slang/slang#11443: `helper`'s primary-file `using namespace Foo;` must
// NOT be re-exported through `import`, so this importer cannot reference `Foo`'s
// members unqualified. Qualified access and an importer-local `using` are still
// allowed and are exercised by using-import-positive.
int useLeaked()
{
    return moduleHelper(1);
//CHECK: undefined identifier 'moduleHelper'
}
