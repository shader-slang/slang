//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// When the dereferenced base of `p.m` has no member `m`, `m` is looked up in the pointer
// types that the base was implicitly dereferenced through. `p->m` names a member of `*p`,
// so the pointer that `->` dereferences does not contribute members. A
// `DescriptorHandle<T>` level never contributes members, even under a pointer.

extension Ptr<int>
{
    int g() { return 1; }
}

extension<T : IOpaqueDescriptor> DescriptorHandle<T>
{
    int h() { return 1; }
}

int arrow(Ptr<int> p)
{
    return p.g() + p->g();
//CHECK:              ^ member not found
//CHECK:              ^ 'g' is not a member of 'int'.
}

int arrowThroughExplicitDeref(Ptr<Ptr<int>> pp)
{
    return pp->g() + (*pp)->g();
//CHECK:                    ^ member not found
//CHECK:                    ^ 'g' is not a member of 'int'.
}

int descriptorHandle(DescriptorHandle<Texture2D> t)
{
    return t.h();
//CHECK:     ^ member not found
//CHECK:     ^ 'h' is not a member of 'Texture2D<vector<float,4>>'.
}

int descriptorHandleThroughPtr(Ptr<DescriptorHandle<Texture2D>> pt)
{
    return pt.h();
//CHECK:      ^ member not found
//CHECK:      ^ 'h' is not a member of 'Texture2D<vector<float,4>>'.
}
