// sizeof.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int func()
{
    return sizeof(func);
//CHECK:   ^^^^^^ invalid sizeof argument
//CHECK:   ^^^^^^ argument to sizeof is invalid
}
