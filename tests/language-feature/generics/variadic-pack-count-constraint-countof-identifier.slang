//TEST:SIMPLE:

struct countof
{
}

void ordinaryIdentifierNamedCountofRight<T>() where T == countof
{
}

void ordinaryIdentifierNamedCountofLeft<T>() where countof == T
{
}

void test()
{
    ordinaryIdentifierNamedCountofRight<countof>();
    ordinaryIdentifierNamedCountofLeft<countof>();
}
