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
    ^^^^ warning
    ^^^^ case 'Mode' has the same name as its unscoped enum
*/
    Other,
};

int useTypeAndValue()
{
    Mode m = Other;
    return (int)m + (int)Mode;
}

// A declaration that the user writes is still an error, even when its value is a case of the
// enum it conflicts with.

[UnscopedEnum]
enum Kind
/*CHECK:
     ^^^^ see previous declaration of 'Kind'
*/
{
    KindCase,
};

let Kind = Kind.KindCase;
/*CHECK:
    ^^^^ conflicting declaration
    ^^^^ declaration of 'Kind' conflicts with existing declaration
*/
