//DIAGNOSTIC_TEST:SIMPLE:

int doSomething(int a)
{
    // Warning can't fit
    int c0 = 0x800000000;
    
    // No warning as top bits are just ignored
    int c1 = -1ll;
    
    int c2 = int(-1u);
    
    // Should sign extend 
    int c3 = 0x80000000;
    
    // Should give a warning (ideally including the preceeding -)
    // Currently we don't have the -, because the lexer lexes - independently
    int c4 = -0xfffffffff;
    
    // 
    a += c0 + c1 + c2;
    
    int64_t b = 0;

    // Ok
    b += 0x800000000ll;
    
    uint64_t c5 = -2ull;
    
    return a + int(b);
}

