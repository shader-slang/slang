//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// User code can attach the unscoped-enum alias marker through `syntax`. Such a declaration names
// no case, so it is not exempt from the conflict with the enum (shader-slang/slang#13876).

syntax my_alias : UnscopedEnumCaseAliasModifier;

[UnscopedEnum]
enum E
/*CHECK:
     ^ see previous declaration of 'E'
*/
{
    A,
};

my_alias static const int E = 1;
/*CHECK:
                          ^ error E30200
                          ^ declaration of 'E' conflicts with existing declaration
*/
