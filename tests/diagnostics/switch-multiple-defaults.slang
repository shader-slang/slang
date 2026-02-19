//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Test to evaluate the behavior of unreachable code blocks within a switch statement. A switch statement with multiple default cases is not allowed and should throw an error

void test()
{
    switch (0)
    {
        default: break;
        default: break;
//diag: ^^^^^^^ multiple default cases in switch
//diag: ^^^^^^^ multiple 'default' cases not allowed within a 'switch' statement
    }
	return;
}
