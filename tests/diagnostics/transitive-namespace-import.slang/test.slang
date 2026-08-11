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
//CHECK:^^^ did you mean 'f_b'?
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
//CHECK:^^^ did you mean 'f_b'?
    }
}

namespace ns3
{
    using namespace ns;

    void testFunc4()
    {
        // A typo of the *non-public* `ns::f_private` from module b. Because
        // lookup would never resolve an inaccessible decl, it must not be
        // offered as a suggestion (exhaustive mode: a spurious "did you mean"
        // would be an unannotated diagnostic and fail the test).
        f_privat(); // Error, no suggestion.
//CHECK:^^^^^^^^ undefined identifier
//CHECK:^^^^^^^^ undefined identifier 'f_privat'.
    }
}
