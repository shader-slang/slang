//TEST:LANG_SERVER(filecheck=CHECK):
//TEST:LANG_SERVER(filecheck=HIDDEN):
//COMPLETE:20,20

// Completion inside a member of a function-local type lists the enclosing function's `static`
// locals and the globals, but not its parameters or non-static locals, which lookup from the
// type does not find. The two runs use separate prefixes because a `CHECK-NOT` between
// `CHECK-DAG` lines only covers the text between their matches.

static const int globalCandidate = 1;

int f(int paramCandidate)
{
    int localCandidate = paramCandidate;
    static const int staticCandidate = 2;
    struct L
    {
        int get()
        {
            return staticCandidate;
        }
    };
    L l;
    return l.get() + localCandidate;
}

// CHECK-DAG: {{^}}staticCandidate:
// CHECK-DAG: {{^}}globalCandidate:
// HIDDEN-NOT: {{^}}localCandidate:
// HIDDEN-NOT: {{^}}paramCandidate:
