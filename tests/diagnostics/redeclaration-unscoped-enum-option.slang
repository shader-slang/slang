//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-unscoped-enum

// With `-unscoped-enum`, every enum adds a `static const` alias for each case to the
// enclosing scope, so a case conflicts with an earlier declaration of the same name
// (shader-slang/slang#13876).

static const int optionCase = 1;
/*CHECK:
                 ^^^^^^^^^^ see previous declaration of 'optionCase'
*/

enum OptionEnum
{
    optionCase = 2,
/*CHECK:
    ^^^^^^^^^^ conflicting declaration
    ^^^^^^^^^^ declaration of 'optionCase' conflicts with existing declaration
*/
};
