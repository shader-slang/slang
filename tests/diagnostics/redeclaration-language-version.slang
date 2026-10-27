//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2018
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026

// Before Slang 202c, a conflict that earlier Slang versions accepted is a warning, and a
// conflict that they reported stays an error (shader-slang/slang#13876). The same declarations
// in Slang 202c are in redeclaration-language-version-202c.slang.

// The reproducer from the issue: the alias of an `[UnscopedEnum]` case follows a
// `static const` of the same name.

static const int caseAfterConst = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^ see previous declaration of 'caseAfterConst'
*/

[UnscopedEnum]
enum EnumAfterConst
{
    caseAfterConst,
/*CHECK:
    ^^^^^^^^^^^^^^ warning E30204
    ^^^^^^^^^^^^^^ declaration of 'caseAfterConst' conflicts with existing declaration
*/
};

// A `cbuffer` member after a `static const` of the same name.

static const int memberAfterConst = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^^ see previous declaration of 'memberAfterConst'
*/

cbuffer BufferAfterConst
{
    int memberAfterConst;
/*CHECK:
        ^^^^^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^^^^^ declaration of 'memberAfterConst' conflicts with existing declaration
*/
}

// A declaration that the user writes gets no exemption, even when its value is a case of the
// unscoped enum that it conflicts with.

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
    ^^^^ warning E30204
    ^^^^ declaration of 'Kind' conflicts with existing declaration
*/

// Earlier Slang versions report these conflicts already, so they are an error in every version.

[UnscopedEnum]
enum EnumBeforeConst
{
    caseBeforeConst,
/*CHECK:
    ^^^^^^^^^^^^^^^ see previous declaration of 'caseBeforeConst'
*/
};

static const int caseBeforeConst = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^ error E30200
                 ^^^^^^^^^^^^^^^ declaration of 'caseBeforeConst' conflicts with existing declaration
*/

void testLocals()
{
    int typedFirst = 1;
/*CHECK:
        ^^^^^^^^^^ see previous declaration of 'typedFirst'
*/
    let typedFirst = 2;
/*CHECK:
        ^^^^^^^^^^ warning E30204
        ^^^^^^^^^^ declaration of 'typedFirst' conflicts with existing declaration
*/

    let inferredFirst = 1;
/*CHECK:
        ^^^^^^^^^^^^^ see previous declaration of 'inferredFirst'
*/
    int inferredFirst = 2;
/*CHECK:
        ^^^^^^^^^^^^^ error E30200
        ^^^^^^^^^^^^^ declaration of 'inferredFirst' conflicts with existing declaration
*/
}
