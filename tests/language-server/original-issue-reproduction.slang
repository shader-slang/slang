//TEST:LANG_SERVER(filecheck=CHECK):
// More accurate reproduction of the original issue - with public

interface ISample
{
    float getValue();
}

struct LambertDiffuseBTDF : ISample
{
    float value;
    
    // Public constructor - should work
    public __init(float val)
    {
        this.value = val;
    }
    
    float getValue() { return value; }
}

LambertDiffuseBTDF createObject(float val)
{
//HOVER:24,11
    return LambertDiffuseBTDF(val);
}

//CHECK: LambertDiffuseBTDF.init