//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

// The line splice follows a complete token. The next token's diagnostic belongs
// at its first physical byte, rather than at the backslash on the preceding line.
int value =\
missingName;
/*CHECK:
^^^^^^^^^^^ undefined identifier
^^^^^^^^^^^ undefined identifier 'missingName'.
*/
