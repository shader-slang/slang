//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A declaration whose type is inferred from its initializer is checked up to
// `DefinitionChecked` in one step. This test confirms that such a declaration is still
// checked for conflicts with earlier declarations in the same scope (shader-slang/slang#13876).

// The parser adds a `static const` alias with an inferred type for each case of an
// `[UnscopedEnum]` enum. The conflict is diagnosed in both declaration orders.

static const int beforeEnum = 1;
/*CHECK:
                 ^^^^^^^^^^ see previous declaration of 'beforeEnum'
*/

[UnscopedEnum]
enum EnumAfterConst
{
    beforeEnum = 2,
/*CHECK:
    ^^^^^^^^^^ warning E30204
    ^^^^^^^^^^ declaration of 'beforeEnum' conflicts with existing declaration
*/
};

[UnscopedEnum]
enum EnumBeforeConst
{
    afterEnum = 1,
/*CHECK:
    ^^^^^^^^^ see previous declaration of 'afterEnum'
*/
};

static const int afterEnum = 2;
/*CHECK:
                 ^^^^^^^^^ error E30200
                 ^^^^^^^^^ declaration of 'afterEnum' conflicts with existing declaration
*/

[UnscopedEnum]
enum FirstEnum
{
    sharedCase,
/*CHECK:
    ^^^^^^^^^^ see previous declaration of 'sharedCase'
*/
};

[UnscopedEnum]
enum SecondEnum
{
    sharedCase,
/*CHECK:
    ^^^^^^^^^^ warning E30204
    ^^^^^^^^^^ declaration of 'sharedCase' conflicts with existing declaration
*/
};

// A repeated case is reported once. The parser adds no alias for it, so the alias of the
// first case does not conflict with it a second time.

[UnscopedEnum]
enum RepeatedCase
{
    repeatedCase,
/*CHECK:
    ^^^^^^^^^^^^ see previous declaration of 'repeatedCase'
*/
    repeatedCase,
/*CHECK:
    ^^^^^^^^^^^^ error E30200
    ^^^^^^^^^^^^ declaration of 'repeatedCase' conflicts with existing declaration
*/
};

// An unsized array takes its size from its initializer in the same step.

static const int sizedFirst = 1;
/*CHECK:
                 ^^^^^^^^^^ see previous declaration of 'sizedFirst'
*/
static const int sizedFirst[] = { 1, 2 };
/*CHECK:
                 ^^^^^^^^^^ warning E30204
                 ^^^^^^^^^^ declaration of 'sizedFirst' conflicts with existing declaration
*/

static const int unsizedFirst[] = { 1, 2 };
/*CHECK:
                 ^^^^^^^^^^^^ see previous declaration of 'unsizedFirst'
*/
static const int unsizedFirst = 1;
/*CHECK:
                 ^^^^^^^^^^^^ error E30200
                 ^^^^^^^^^^^^ declaration of 'unsizedFirst' conflicts with existing declaration
*/

// A global `let` with an inferred type.

static const int typedThenLet = 1;
/*CHECK:
                 ^^^^^^^^^^^^ see previous declaration of 'typedThenLet'
*/
let typedThenLet = 2;
/*CHECK:
    ^^^^^^^^^^^^ warning E30204
    ^^^^^^^^^^^^ declaration of 'typedThenLet' conflicts with existing declaration
*/

// Local variables.

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

    var inferredVar = 1;
/*CHECK:
        ^^^^^^^^^^^ see previous declaration of 'inferredVar'
*/
    let inferredVar = 2;
/*CHECK:
        ^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^ declaration of 'inferredVar' conflicts with existing declaration
*/

    // A declaration in a nested scope shadows the outer one instead of conflicting with it.
    let shadowed = 1;
    {
        let shadowed = 2;
    }
}
