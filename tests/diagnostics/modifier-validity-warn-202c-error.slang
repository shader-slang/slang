// Companion to `modifier-validity-warn.slang`: the same meaningless `triangle`-on-a-struct usage
// that is a lenient warning (E31232) in the current language version becomes a hard error (E31201)
// under the 202c language rules, because its rule's disposition escalates with the language
// version. Exhaustive `diag` mode binds the annotation to the `triangle` token, so a spurious
// extra diagnostic anywhere would fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target spirv -std 202c

triangle struct MeaninglessQualifier
/*diag:
^^^^^^^^ modifier not allowed
^^^^^^^^ modifier 'triangle' is not allowed here.
*/
{
    int x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    MeaninglessQualifier v;
    v.x = 0;
}
