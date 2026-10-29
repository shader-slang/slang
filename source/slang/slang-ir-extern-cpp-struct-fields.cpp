// slang-ir-extern-cpp-struct-fields.cpp
#include "slang-ir-extern-cpp-struct-fields.h"

#include "slang-ir-insts.h"

namespace Slang
{

/// Return `base` followed by the smallest number that gives a name not in `usedNames`.
///
/// The number follows an underscore unless `base` already ends with one: `base` gives `base_0`,
/// or `base_1` when a field already uses `base_0`.
static String makeUnusedFieldName(UnownedStringSlice base, const HashSet<String>& usedNames)
{
    for (Int32 n = 0;; n++)
    {
        StringBuilder sb;
        sb << base;
        if (!base.endsWith("_"))
            sb << "_";
        sb << n;
        String candidate = sb.produceString();
        if (!usedNames.contains(candidate))
            return candidate;
    }
}

void applyExternCppStructFieldNames(IRModule* module)
{
    IRBuilder builder(module);
    for (auto inst : module->getGlobalInsts())
    {
        auto structType = as<IRStructType>(inst);
        if (!structType || !structType->findDecoration<IRExternCppDecoration>())
            continue;

        // We choose every member name of the struct here, so that the names are distinct by
        // construction. Names of fields marked `__extern_cpp` themselves are fixed already.
        HashSet<String> usedNames;
        Dictionary<String, Index> fieldCountForNameHint;
        for (auto field : structType->getFields())
        {
            auto key = field->getKey();
            if (auto externCpp = key->findDecoration<IRExternCppDecoration>())
                usedNames.add(String(externCpp->getName()));
            else if (auto nameHint = key->findDecoration<IRNameHintDecoration>())
                fieldCountForNameHint.getOrAddValue(nameHint->getName(), 0)++;
        }

        // A field keeps its source name unless the checker synthesized it (such members are
        // named with a leading `$`, which no source identifier contains and C++ rejects) or
        // another field shares the name. Lowering names the field that holds an inherited struct
        // `base`, so a source field `base` in a derived struct shares that name.
        List<IRInst*> keysToNumber;
        for (auto field : structType->getFields())
        {
            auto key = field->getKey();
            if (key->findDecoration<IRExternCppDecoration>())
                continue;

            // Name hints are stripped under `-obfuscate`; such a field keeps its hashed name.
            auto nameHint = key->findDecoration<IRNameHintDecoration>();
            if (!nameHint)
                continue;

            auto name = nameHint->getName();
            if (name.startsWith("$") || fieldCountForNameHint.getValue(name) > 1 ||
                usedNames.contains(String(name)))
            {
                keysToNumber.add(key);
                continue;
            }
            builder.addExternCppDecoration(key, name);
            usedNames.add(String(name));
        }

        // The remaining fields get their name hint, without a leading `$`, plus a number that no
        // other member of the struct uses.
        for (auto key : keysToNumber)
        {
            auto name = key->findDecoration<IRNameHintDecoration>()->getName();
            if (name.startsWith("$"))
                name = name.tail(1);
            auto uniqueName = makeUnusedFieldName(name, usedNames);
            builder.addExternCppDecoration(key, uniqueName.getUnownedSlice());
            usedNames.add(uniqueName);
        }
    }
}

} // namespace Slang
