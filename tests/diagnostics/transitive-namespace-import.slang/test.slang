//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

module test;

import b;

void testFunc()
{
    ns.f_b(); // OK - no error expected here
    ns.f_a(); // Error.
/*CHECK:
       ^^^ member not found
       ^^^ 'f_a' is not a member of 'overload group'.
*/
}

namespace ns
{
    void testFunc2()
    {
        f_b(); // OK - no error expected here
        f_a(); // Error.
//CHECK:^^^ undefined identifier
//CHECK:^^^ undefined identifier 'f_a'.
    }
}

namespace ns2
{
    using namespace ns;

    void testFunc3()
    {
        f_b(); // OK - no error expected here
        f_a(); // Error.
//CHECK:^^^ undefined identifier
//CHECK:^^^ undefined identifier 'f_a'.
    }
}
