//TEST_IGNORE_FILE:

// obfuscated-loc-module.slang

// This function is designed to fail during IR passes/emit. 
int silly(int a)
{
    int t = 0;
    [ForceUnroll(10)]
    while ( a > 0)
    {
        t = t + t + a;
    }
    return t;
}