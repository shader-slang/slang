//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2018
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 202c

// A case of an `[UnscopedEnum]` enum with the same name as its enum gets a warning in every
// language version (shader-slang/slang#13876).

[UnscopedEnum]
enum Mode
{
    Mode,
/*CHECK:
    ^^^^ warning E30203
    ^^^^ case 'Mode' has the same name as its unscoped enum
*/
    Other,
};

int useTypeAndValue()
{
    Mode m = Other;
    return (int)m + (int)Mode;
}

// The warning ends the comparison of the case alias with earlier declarations, so an earlier
// `static const` of the same name conflicts with the enum, not with the alias.

static const int Level = 0;
/*CHECK:
                 ^^^^^ see previous declaration of 'Level'
*/

[UnscopedEnum]
enum Level
/*CHECK:
     ^^^^^ error E30200
     ^^^^^ declaration of 'Level' conflicts with existing declaration
*/
{
    Level,
/*CHECK:
    ^^^^^ warning E30203
    ^^^^^ case 'Level' has the same name as its unscoped enum
*/
};
