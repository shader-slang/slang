//TEST(smoke):LANG_SERVER:

void bar()
{
    foo(1, 2.0);
//HOVER:5,5
}

void foo(int i = 0, float j = 1.0)
{
}
