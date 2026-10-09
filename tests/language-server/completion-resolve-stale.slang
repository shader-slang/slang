//TEST:LANG_SERVER:

// completionItem/resolve resolves an item against the completion request that produced it. An item
// from an earlier completion request must not be resolved against the declarations of a later one:
// `beta` below is the second member of A, but the second member of B is a different declaration.

struct A { int alpha; float beta; };
struct B { double gamma; int delta; };

void test(A a, B b)
{
    //COMPLETE:17,7
    //SAVE_COMPLETION_ITEM:beta
    //RESOLVE_SAVED_ITEM:
    //COMPLETE:18,7
    //RESOLVE_SAVED_ITEM:
    a.;
    b.;
}
