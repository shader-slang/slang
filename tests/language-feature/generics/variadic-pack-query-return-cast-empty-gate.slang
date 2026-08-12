//TEST:SIMPLE(filecheck=CHECK):

// Companion to variadic-pack-query-return-cast.slang (shader-slang/slang#12494): the fix must not
// weaken the empty-pack gate. Without `where nonempty`, applying `__first`/`__last` to a value pack
// of the reported `expand each T args` shape must still be an error, by diagnostic CODE (E30412),
// not merely by message text.

//CHECK-DAG: error[E30412]{{.*}}__first
int firstNoGate<each T>(expand each T args) where T == int
{
    return __first(args);
}

//CHECK-DAG: error[E30412]{{.*}}__last
int lastNoGate<each T>(expand each T args) where T == int
{
    return __last(args);
}
