module iface;

namespace m {

public interface IFoo {
    int get_val();
}

public struct Entry {
    public uint header;
    public uint blob;
}

public struct System {
    public StructuredBuffer<Entry> entries;
    public IFoo get_foo(uint id) {
        return createDynamicObject<IFoo>(entries[id].header, entries[id]);
    }
}

}
