// The `__ref`, `__constref`, `__borrow`, `__ref_readonly` and `__ref_writeonly`
// modifiers describe parameter-passing modes, so the checker permits them on
// parameters only. Applying any of them to a global variable, with or without
// `static`, is rejected with error 31201, matching the existing rule that
// rejects these modifiers on struct fields.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

__ref int refGlobal;
/*CHECK:
^^^^^ E31201
^^^^^ modifier '__ref' is not allowed here.
*/

__constref float constrefGlobal;
/*CHECK:
^^^^^^^^^^ E31201
^^^^^^^^^^ modifier '__constref' is not allowed here.
*/

__borrow int borrowGlobal;
/*CHECK:
^^^^^^^^ E31201
^^^^^^^^ modifier '__borrow' is not allowed here.
*/

__ref_readonly int refReadonlyGlobal;
/*CHECK:
^^^^^^^^^^^^^^ E31201
^^^^^^^^^^^^^^ modifier '__ref_readonly' is not allowed here.
*/

__ref_writeonly int refWriteonlyGlobal;
/*CHECK:
^^^^^^^^^^^^^^^ E31201
^^^^^^^^^^^^^^^ modifier '__ref_writeonly' is not allowed here.
*/

static __ref int staticRefGlobal;
/*CHECK:
       ^^^^^ E31201
       ^^^^^ modifier '__ref' is not allowed here.
*/

static __constref float staticConstrefGlobal;
/*CHECK:
       ^^^^^^^^^^ E31201
       ^^^^^^^^^^ modifier '__constref' is not allowed here.
*/

static __borrow int staticBorrowGlobal;
/*CHECK:
       ^^^^^^^^ E31201
       ^^^^^^^^ modifier '__borrow' is not allowed here.
*/

static __ref_readonly int staticRefReadonlyGlobal;
/*CHECK:
       ^^^^^^^^^^^^^^ E31201
       ^^^^^^^^^^^^^^ modifier '__ref_readonly' is not allowed here.
*/

static __ref_writeonly int staticRefWriteonlyGlobal;
/*CHECK:
       ^^^^^^^^^^^^^^^ E31201
       ^^^^^^^^^^^^^^^ modifier '__ref_writeonly' is not allowed here.
*/

[numthreads(1, 1, 1)]
void computeMain()
{
}
