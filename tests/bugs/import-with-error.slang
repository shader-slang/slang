//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Confirm that we correctly issue a diagnostic when
// we `import` a module that has some errors in it.

import import_with_error_extra;
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^ import of module 'import_with_error_extra' failed because of a compilation error
*/

void main()
{
	broken();
}
