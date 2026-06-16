implementing nested_reopen_ns;

namespace outer {
namespace repro {

// Unqualified `IBase`/`ICollector`/`Add` here must resolve to the declarations in
// nested_reopen_ns_types (a different fragment reopening `outer::repro`). This is
// what exercises the outermost-first wiring of the nested namespaces.
public extension<In1 : IBase, In2 : IBase> Add<In1, In2>
    : ICollector
    where In1 : ICollector
    where In2 : ICollector
{
    public int collect()
    {
        return this.in1.collect() + this.in2.collect();
    }
}

} // namespace repro
} // namespace outer
