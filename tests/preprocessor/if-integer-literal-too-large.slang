// An integer literal too large for any integer type is an error in `#if`, as it is in code.
// A skipped `#if` arm is not evaluated, so its literals are not diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#if 18446744073709551616 > 0
/*CHECK:
    ^^^^^^^^^^^^^^^^^^^^ error E10012
*/
#endif

#if 0
#if 18446744073709551616 > 0
#endif
#endif

void main() {}
