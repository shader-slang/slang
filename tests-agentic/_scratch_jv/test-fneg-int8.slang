//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    uint64_t x = 99999999999999999999;
//CHECK: integer literal is too large
}
