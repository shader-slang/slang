#include "../../lib/api/spvdb.h"
#include "../../lib/api/spvdb_session.h"
#include <cstring>
#include <iostream>

using namespace spvdb;

int main()
{
    auto mod = load_module_from_file("tests/slang/add_buffers.spv");
    if (\!mod) {
        std::cerr << "load: " << mod.error().message << "\n";
        return 1;
    }

    auto sess = create_session(*mod, "main");
    if (\!sess) {
        std::cerr << "session: " << sess.error().message << "\n";
        return 1;
    }

    set_descriptor_json(**sess, 0, 0, "[7]");
    set_descriptor_json(**sess, 0, 1, "[5]");
    set_descriptor_json(**sess, 0, 2, "[0]");

    auto reason = run(**sess);
    std::cout << "StopReason: " << (int) reason << "\n";
    if (reason == StopReason::Panic)
        std::cerr << "Panic: " << panic_message(**sess) << "\n";

    // Diagnostics
    for (auto &d : (*sess)->interp.diagnostics) std::cout << "diag: " << d << "\n";

    // Output
    auto outs = output_variables(**sess);
    std::cout << "outputs: " << outs.size() << "\n";
    for (auto &v : outs) std::cout << "  " << v.name << "\n";

    // Read back binding 0:2
    auto r = read_descriptor(**sess, 0, 2);
    if (r) {
        uint32_t val;
        std::memcpy(&val, r->data(), 4);
        std::cout << "output[0] = " << val << "\n";
    }

    // Local variables
    auto locals = local_variables(**sess);
    std::cout << "locals: " << locals.size() << "\n";
    for (auto &v : locals) std::cout << "  " << v.name << "\n";

    return 0;
}
