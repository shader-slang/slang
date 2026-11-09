//DIAGNOSTIC_TEST:SIMPLE(diag=FIRST): -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=SECOND): -no-codegen

// Both runs diagnose the same source. Annotations belonging to the other run must not be
// mistaken for source lines when locating a diagnostic. Exercise line and block annotations
// in both prefix orders.
void testLineAnnotations()
{
    int value = missingLine;
//FIRST:        ^^^^^^^^^^^ undefined identifier
//FIRST:        ^^^^^^^^^^^ undefined identifier 'missingLine'.
//SECOND:       ^^^^^^^^^^^ undefined identifier
//SECOND:       ^^^^^^^^^^^ undefined identifier 'missingLine'.
}

void testBlockAnnotations()
{
    missingBlock();
/*SECOND:
    ^^^^^^^^^^^^ undefined identifier
    ^^^^^^^^^^^^ undefined identifier 'missingBlock'.
*/
/*FIRST:
    ^^^^^^^^^^^^ undefined identifier
    ^^^^^^^^^^^^ undefined identifier 'missingBlock'.
*/
}
