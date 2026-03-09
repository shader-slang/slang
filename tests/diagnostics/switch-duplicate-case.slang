//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Tests to evaluate the behavior of code blocks within a switch statement. A switch statement with duplicate cases with same values is not allowed and should throw an error

enum class Cases
{
    A,
    B
};

void test1(Cases c)
{
    switch (c)
    {
        case Cases::A: break;
        case Cases::B: break;
        case Cases::A: break;
//diag: ^^^^ duplicate cases in switch
//diag: ^^^^ duplicate cases not allowed within a 'switch' statement
    }
    return;
}

void test2()
{
    switch (0)
    {
        case 1: break;
        case 2: break;
        case 1: break;
//diag: ^^^^ duplicate cases in switch
//diag: ^^^^ duplicate cases not allowed within a 'switch' statement
    }
	return;
}
