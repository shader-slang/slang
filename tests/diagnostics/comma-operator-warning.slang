//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int testCommaOperator()
{
    // This should generate a warning - comma operator in variable initialization
    float4 vColor = (0.f, 0.f, 0.f, 1.f);
//CHECK:             ^^^ expression cannot be used as a statement
//CHECK:             ^^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
//CHECK:             ^^^ result of this expression is not used
//CHECK:             ^^^ the result of this expression is not used; consider binding it to a variable with 'let'.
//CHECK:                ^ expression cannot be used as a statement
//CHECK:                ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
//CHECK:                ^ comma operator used in expression
//CHECK:                ^ comma operator used in expression (may be unintended)
//CHECK:                  ^^^ result of this expression is not used
//CHECK:                  ^^^ the result of this expression is not used; consider binding it to a variable with 'let'.
//CHECK:                     ^ expression cannot be used as a statement
//CHECK:                     ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
//CHECK:                     ^ comma operator used in expression
//CHECK:                     ^ comma operator used in expression (may be unintended)
//CHECK:                       ^^^ result of this expression is not used
//CHECK:                       ^^^ the result of this expression is not used; consider binding it to a variable with 'let'.
//CHECK:                          ^ comma operator used in expression
//CHECK:                          ^ comma operator used in expression (may be unintended)

    // This should NOT generate a warning - comma operator in for-loop side effect
    for (int i = 0; i < 10; i++, vColor.x++)
    {
    }

    // This should generate a warning - comma operator in regular expression
    int x = (1, 2, 3);
//CHECK:     ^ expression cannot be used as a statement
//CHECK:     ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
//CHECK:     ^ result of this expression is not used
//CHECK:     ^ the result of this expression is not used; consider binding it to a variable with 'let'.
//CHECK:      ^ expression cannot be used as a statement
//CHECK:      ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
//CHECK:      ^ comma operator used in expression
//CHECK:      ^ comma operator used in expression (may be unintended)
//CHECK:        ^ result of this expression is not used
//CHECK:        ^ the result of this expression is not used; consider binding it to a variable with 'let'.
//CHECK:         ^ comma operator used in expression
//CHECK:         ^ comma operator used in expression (may be unintended)

    // This should now generate a warning - comma operator in return statement
    int a = 5;
    return a *= 2, a + 1;
//CHECK:         ^ comma operator used in expression
//CHECK:         ^ comma operator used in expression (may be unintended)
}

void someFunction(int value) {}