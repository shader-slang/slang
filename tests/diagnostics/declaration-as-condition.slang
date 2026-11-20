//DIAGNOSTIC_TEST:SIMPLE(diag=SLANG): -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=GLSL): -lang glsl tests/diagnostics/declaration-as-condition.slang -DGLSL_TEST -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SCOPE): -lang hlsl tests/diagnostics/declaration-as-condition.slang -DSCOPE_TEST -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=DO_WHILE): -lang hlsl tests/diagnostics/declaration-as-condition.slang -DDO_WHILE_TEST -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=DECLARATOR): -lang hlsl tests/diagnostics/declaration-as-condition.slang -DDECLARATOR_TEST -no-codegen

#if defined(SCOPE_TEST)
void readAfterIf()
{
    if (bool selected = true)
    {}
    bool value = selected;
/*SCOPE:
                 ^^^^^^^^ undefined identifier
                 ^^^^^^^^ undefined identifier 'selected'
*/
}

void readAfterWhile()
{
    while (bool repeat = false)
    {}
    bool value = repeat;
/*SCOPE:
                 ^^^^^^ undefined identifier
                 ^^^^^^ undefined identifier 'repeat'
*/
}
#elif defined(DO_WHILE_TEST)
void declarationInDoWhileCondition()
{
    do
    {}
    while (bool repeat = false);
/*DO_WHILE:
           ^^^^ a variable declaration is not allowed as a condition
*/
}
#elif defined(DECLARATOR_TEST)
void noInitializer()
{
    if (bool enabled)
/*DECLARATOR:
                    ^ unexpected token
                    ^ unexpected ')', expected '='
*/
    {}
}

void extraDeclarator()
{
    if (bool first = false, second = true)
/*DECLARATOR:
                          ^ unexpected token
                          ^ unexpected ',', expected ')'
*/
    {}
}
#elif defined(GLSL_TEST)
void declarationInIfCondition()
{
    if (bool selected = true)
/*GLSL:
        ^^^^ a variable declaration is not allowed as a condition
*/
    {}
}
#else
void declarationsInConditions()
{
    if (bool selected = true)
/*SLANG:
        ^^^^ a variable declaration is not allowed as a condition
*/
    {}

    while (bool repeat = false)
/*SLANG:
           ^^^^ a variable declaration is not allowed as a condition
*/
    {}
}
#endif
