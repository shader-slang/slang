//DIAGNOSTIC_TEST:SIMPLE:


float doSomething(float a)
{
    // Too large with become FLT_MAX
    a += 5e+40;
    // Will be narrowed to 0
    a += 9e-50;
    
    double b = 0.0f;
    
    // These shouldn't produce warning as they can fit
    b += 5e+40l;
    b += 9e-50l;
    
    return a + float(b);
}
