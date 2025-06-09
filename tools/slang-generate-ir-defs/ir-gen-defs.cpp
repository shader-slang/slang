#include "ir-yaml-parser.h"

#include <core/slang-io.h>

using namespace Slang;


static String flagsToString(IROpFlags flags)
{
    if (flags == IROpFlags::None)
        return "0";

    List<String> flag_names;
    if ((flags & IROpFlags::Parent) != IROpFlags::None)
        flag_names.add("kIROpFlag_Parent");
    if ((flags & IROpFlags::UseOther) != IROpFlags::None)
        flag_names.add("kIROpFlag_UseOther");
    if ((flags & IROpFlags::Hoistable) != IROpFlags::None)
        flag_names.add("kIROpFlag_Hoistable");
    if ((flags & IROpFlags::Global) != IROpFlags::None)
        flag_names.add("kIROpFlag_Global");

    StringBuilder result;
    for (size_t i = 0; i < flag_names.getCount(); ++i)
    {
        if (i > 0)
            result << " | ";
        result << flag_names[i];
    }
    return result;
}

// Process entry for enum generation
static void processEntryForEnum(
    const Entry& entry,
    IROpFlags inherited_flags,
    StringBuilder& output,
    bool& isFirst,
    String& firstInRange,
    String& lastInRange)
{
    if (entry.isInstruction())
    {
        // Add comment if present
        if (entry.comment.getLength())
        {
            output << "    /* " << entry.comment << " */\n";
        }

        output << "    kIROp_" << entry.type_name;
        if (isFirst)
        {
            output << " = 0";
        }
        output << ",\n";


        if (firstInRange.getLength() == 0)
            firstInRange = entry.type_name;
        lastInRange = entry.type_name;

        isFirst = false;
    }
    else // It's a range
    {
        IROpFlags combined_flags = entry.flags | inherited_flags;
        String rangeFirst, rangeLast;

        // Process nested entries
        for (const auto& nested : entry.children)
        {
            processEntryForEnum(nested, combined_flags, output, isFirst, rangeFirst, rangeLast);
        }

        // Generate range constants if we processed any instructions
        if (rangeFirst.getLength() > 0)
        {
            output << "    kIROp_First" << entry.name << " = kIROp_" << rangeFirst << ",\n";
            output << "    kIROp_Last" << entry.name << " = kIROp_" << rangeLast << ",\n";

            // Update parent range tracking
            if (firstInRange.getLength() == 0)
                firstInRange = rangeFirst;
            lastInRange = rangeLast;
        }
    }
}

// Process entry for info table generation
static void processEntryForInfo(
    const Entry& entry,
    IROpFlags inherited_flags,
    StringBuilder& output)
{
    if (entry.isInstruction())
    {
        IROpFlags combined_flags = entry.flags | inherited_flags;
        output << "    {kIROp_" << entry.type_name << ",\n";
        output << "     {\n";
        output << "         \"" << entry.name << "\",\n";
        output << "         " << entry.operands << ",\n";
        output << "         " << flagsToString(combined_flags) << ",\n";
        output << "     }},\n";
    }
    else // It's a range
    {
        IROpFlags combined_flags = entry.flags | inherited_flags;

        // Process nested entries
        for (const auto& nested : entry.children)
        {
            processEntryForInfo(nested, combined_flags, output);
        }
    }
}

static String generateEnum(const InstructionSet& inst_set)
{
    StringBuilder output;

    // Generate header
    output << R"(#pragma once
// Auto-generated file - do not edit directly

#pragma once

#include <cstdint>

namespace Slang {

/* Bit usage of IROp is as follows

          MainOp | Other
Bit range: 0-10   | Remaining bits

For doing range checks (for example for doing isa tests), the value is masked by kIROpMask_OpMask,
such that the Other bits don't interfere. The other bits can be used for storage for anything that
needs to identify as a different 'op' or 'type'. It is currently used currently for storing the
TextureFlavor of a IRResourceTypeBase derived types for example.

TODO: We should eliminate the use of the "other" bits so that the entire value/state
of an instruction is manifest in its opcode, operands, and children.
*/
enum IROp : int32_t
{
)";

    // Process all entries
    bool isFirst = true;
    for (const auto& entry : inst_set.entries)
    {
        String dummy1, dummy2;
        processEntryForEnum(entry, IROpFlags::None, output, isFirst, dummy1, dummy2);
    }

    output << R"(
    /// The total number of valid opcodes
    kIROpCount,

    /// An invalid opcode used to represent a missing or unknown opcode value.
    kIROp_Invalid = kIROpCount,
};
} // namespace Slang
)";

    return output;
}

static String generateInfoTable(const InstructionSet& inst_set)
{
    StringBuilder output;

    output << R"(// slang-ir-opcodes-info.cpp
// Auto-generated file - do not edit directly

#include "slang-ir.h"
#include "core/slang-basic.h"

namespace Slang {

struct IROpMapEntry
{
    IROp op;
    IROpInfo info;
};

// TODO: We should ideally be speeding up the name->inst
// mapping by using a dictionary, or even by pre-computing
// a hash table to be stored as a `static const` array.
//
// NOTE! That this array is now constructed in such a way that looking up
// an entry from an op is fast, by keeping blocks of main, and pseudo ops in same order
// as the ops themselves. Care must be taken to keep this constraint.
static const IROpMapEntry kIROps[] = {

// Main ops in order
)";

    // Process all entries
    for (const auto& entry : inst_set.entries)
    {
        processEntryForInfo(entry, IROpFlags::None, output);
    }

    output << R"(
    // Invalid op sentinel value comes after all the valid ones
    {kIROp_Invalid, {"invalid", 0, 0}},
};

IROpInfo getIROpInfo(IROp opIn)
{
    const int op = opIn & kIROpMask_OpMask;
    if (op < kIROpCount)
    {
        // It's a main op
        const auto& entry = kIROps[op];
        SLANG_ASSERT(entry.op == op);
        return entry.info;
    }

    // Don't know what this is
    SLANG_ASSERT(!"Invalid op");
    SLANG_ASSERT(kIROps[kIROpCount].op == kIROp_Invalid);
    return kIROps[kIROpCount].info;
}

IROp findIROp(const UnownedStringSlice& name)
{
    for (auto ee : kIROps)
    {
        if (name == ee.info.name)
            return ee.op;
    }

    return IROp(kIROp_Invalid);
}

} // namespace Slang
)";

    return output;
}

int main(int argc, char* argv[])
{
    String input_file;
    String enum_output_file;
    String info_output_file;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        String arg = argv[i];

        if (arg == "--ir-enum-file" && i + 1 < argc)
        {
            enum_output_file = argv[++i];
        }
        else if (arg == "--ir-info-file" && i + 1 < argc)
        {
            info_output_file = argv[++i];
        }
        else if (input_file.getLength() == 0)
        {
            input_file = arg;
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if (input_file.getLength() == 0 && enum_output_file.getLength() == 0)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <input.yaml> [--ir-enum-file <output.h>] [--ir-info-file <output.cpp>]\n";
        return 1;
    }

    try
    {
        String contents;
        if (!SLANG_SUCCEEDED(File::readAllText(input_file, contents)))
        {
            std::cerr << "Error: Cannot open input file '" << input_file << "'\n";
            return 1;
        }

        // Parse YAML
        InstructionSet inst_set = parseInstDefs(input_file, contents);

        // Write enum header file
        if (enum_output_file.getLength())
        {
            String enum_output = generateEnum(inst_set);
            if (!SLANG_SUCCEEDED(File::writeAllText(enum_output_file, enum_output)))
            {
                std::cerr << "Error: Cannot create output file '" << enum_output_file << "'\n";
                return 1;
            }
        }

        // Generate info table if requested
        if (info_output_file.getLength() > 0)
        {
            String info_output = generateInfoTable(inst_set);

            if (!SLANG_SUCCEEDED(File::writeAllText(info_output_file, info_output)))
            {
                std::cerr << "Error: Cannot create output file '" << info_output_file << "'\n";
                return 1;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
