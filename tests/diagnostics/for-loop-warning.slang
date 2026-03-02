//DIAGNOSTIC_TEST:SIMPLE(diag=diag):


float doSomething(int x)
{
    for (int i = 0; i < x; i--) // warn.
//diag:                     ^^ for loop modifies variable in wrong direction
//diag:                     ^^ the for loop is modifiying variable 'i' in the opposite direction from loop exit condition.
    {}
    for (int i = 0; i < 5; i-=-2) // ok.
    {}
    for (int j = 0; j < 3; j += 0) // warn.
//diag:                      ^^ for loop step is zero
//diag:                      ^^ the for loop is not modifiying variable 'j' because the step size evaluates to 0.
    {}
    for (int i = 0; i < 5; i++) // ok
    {
        for (int j = 0; j < 3; i++) // warn.
//diag:                        ^ for loop modifies wrong variable
//diag:                        ^ the for loop initializes and checks variable 'j' but the side effect expression is modifying 'i'.
        {}
    }
    for (int i = 0; i < 5; i++) // ok
    {
        for (int j = 0; i < 4; j++) // warn.
//diag:                   ^ for loop predicate checks wrong variable
//diag:                   ^ the for loop initializes and modifies variable 'j' but the predicate expression is checking 'i'.
        {}
    }

    [MaxIters(6)] // warn
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 3 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 0; i <= 6; i+=3)
    {
    }

    [MaxIters(6)] // warn
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 2 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 5; i >= 0; i -= 3)
    {
    }
    [MaxIters(6)] // warn
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 5; i > 0; i--)
    {
    }

    [MaxIters(5)] // ok
    for (int i = 0; i < 5; i++) // ok
    {
    }

    for (int i = 1; i < 0; i++) // warn
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/
    {
    }
    for (int i = 1; i >= 2; i--) // warn
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/
    {
    }
    for (int i = 1; i >= 1; i--) // ok
    {
    }
    for (int i = 1; i > 1; i--) // warn
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/
    {
    }
    [MaxIters(5)] // ok, because the loop body modifies i so we can't infer the iterations.
    for (int i = 0; i < 5; i+=2)
    {
        i--;
    }
    return 0.0;
}
