// slang-ir-extern-cpp-struct-fields.cpp
#include "slang-ir-extern-cpp-struct-fields.h"

#include "slang-ir-insts.h"

namespace Slang
{

/// Return `prefix` followed by the smallest number that gives a name not in `usedNames`.
///
/// The number follows an underscore unless `prefix` already ends with one: `pos` gives `pos_0`,
/// or `pos_1` when a field already uses `pos_0`.
static String makeUnusedFieldName(UnownedStringSlice prefix, const HashSet<String>& usedNames)
{
    for (Int32 n = 0;; n++)
    {
        StringBuilder sb;
        sb << prefix;
        if (!prefix.endsWith("_"))
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

        // We name every field that has a name hint here, so that those names are distinct from
        // each other. Names of fields marked `__extern_cpp` themselves are fixed already.
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
        // named with a leading `$`, which no source identifier contains and is not standard C++) or
        // another field shares the name. Lowering names the field that holds an inherited struct
        // `base`, so a source field `base` in a derived struct shares that name.
        List<KeyValuePair<IRInst*, UnownedStringSlice>> fieldsToNumber;
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
                fieldsToNumber.add(KeyValuePair<IRInst*, UnownedStringSlice>(
                    key,
                    name.startsWith("$") ? name.tail(1) : name));
                continue;
            }
            builder.addExternCppDecoration(key, name);
            usedNames.add(String(name));
        }

        for (auto& field : fieldsToNumber)
        {
            auto uniqueName = makeUnusedFieldName(field.value, usedNames);
            builder.addExternCppDecoration(field.key, uniqueName.getUnownedSlice());
            usedNames.add(uniqueName);
        }
    }
}

} // namespace Slang
