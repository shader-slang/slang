module type_b;
import iface;

namespace m {

public struct ConcreteB : IFoo {
    public uint header;
    public uint payload_b;
    public int get_val() { return -int(payload_b); }
}

}
