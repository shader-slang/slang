// slang-ir-extern-cpp-struct-fields.cpp
#include "slang-ir-extern-cpp-struct-fields.h"

#include "slang-ir-insts.h"

namespace Slang
{

void applyExternCppStructFieldNames(IRModule* module)
{
    IRBuilder builder(module);
    for (auto inst : module->getGlobalInsts())
    {
        auto structType = as<IRStructType>(inst);
        if (!structType || !structType->findDecoration<IRExternCppDecoration>())
            continue;

        for (auto field : structType->getFields())
        {
            auto key = field->getKey();

            // A field marked `__extern_cpp` itself already has its C++ name.
            if (key->findDecoration<IRExternCppDecoration>())
                continue;

            // Name hints are stripped under `-obfuscate`; such a field keeps its hashed name.
            auto nameHint = key->findDecoration<IRNameHintDecoration>();
            if (!nameHint)
                continue;

            // The lexer never produces `$` in an identifier, so a `$`-prefixed name belongs to
            // a member that the checker synthesized (such as bitfield backing storage). It has no
            // source name to keep, and `$` is not valid in a C++ identifier.
            auto name = nameHint->getName();
            if (name.startsWith("$"))
                continue;

            // The key keeps its qualified linkage name, so same-named fields of different
            // structs remain distinct keys; only the emitted name changes.
            builder.addExternCppDecoration(key, name);
        }
    }
}

} // namespace Slang
