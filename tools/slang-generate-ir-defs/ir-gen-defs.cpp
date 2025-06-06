#include "ir-yaml-parser.h"

#include <core/slang-io.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace Slang;

class IREnumGenerator
{
private:
    struct InstructionInfo
    {
        String name;
        String type_name;
        String comment;
        int operands;
        IROpFlags flags;
    };

    struct RangeInfo
    {
        String name;
        String first_type;
        String last_type;
    };

    std::vector<InstructionInfo> instructions;
    std::vector<RangeInfo> ranges;

    void collectEntry(const Entry& entry, IROpFlags inherited_flags)
    {
        if (entry.isInstruction())
        {
            InstructionInfo info;
            info.name = entry.name;
            info.type_name = entry.type_name;
            info.comment = entry.comment;
            info.operands = entry.operands;
            info.flags = entry.flags | inherited_flags;
            instructions.push_back(info);
        }
        else // It's a range
        {
            // Process nested entries first
            IROpFlags combined_flags = entry.flags | inherited_flags;
            size_t start_index = instructions.size();

            for (const auto& nested : entry.children)
            {
                collectEntry(nested, combined_flags);
            }

            // Add range info if we added any instructions
            if (instructions.size() > start_index)
            {
                RangeInfo range;
                range.name = entry.name;
                range.first_type = instructions[start_index].type_name;
                range.last_type = instructions.back().type_name;
                ranges.push_back(range);
            }
        }
    }

public:
    std::string generate(const InstructionSet& inst_set)
    {
        // Collect all instructions and ranges
        instructions.clear();
        ranges.clear();

        for (const auto& entry : inst_set.entries)
        {
            collectEntry(entry, IROpFlags::None);
        }

        std::ostringstream output;

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

        // Generate enum values
        for (size_t i = 0; i < instructions.size(); ++i)
        {
            const auto& inst = instructions[i];

            // Add comment if present
            if (inst.comment.getLength() > 0)
            {
                output << "    /* " << inst.comment << " */\n";
            }

            output << "    kIROp_" << inst.type_name;
            if (i == 0)
            {
                output << " = 0";
            }
            output << ",\n";
        }

        output << R"(
    /// The total number of valid opcodes
    kIROpCount,

    /// An invalid opcode used to represent a missing or unknown opcode value.
    kIROp_Invalid = kIROpCount,
)";

        // Generate range constants
        if (!ranges.empty())
        {
            output << "\n    // Range constants\n";
            for (const auto& range : ranges)
            {
                output << "    kIROp_First" << range.name << " = kIROp_" << range.first_type
                       << ",\n";
                output << "    kIROp_Last" << range.name << " = kIROp_" << range.last_type << ",\n";
            }
        }

        output << R"(};
} // namespace Slang
)";

        return output.str();
    }

    std::string generateInfoTable(const InstructionSet& inst_set)
    {
        // Collect all instructions
        instructions.clear();
        ranges.clear();

        for (const auto& entry : inst_set.entries)
        {
            collectEntry(entry, IROpFlags::None);
        }

        std::ostringstream output;

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

        // Generate info table entries
        for (const auto& inst : instructions)
        {
            output << "    {kIROp_" << inst.type_name << ",\n";
            output << "     {\n";
            output << "         \"" << inst.name << "\",\n";
            output << "         " << inst.operands << ",\n";

            // Convert flags to string representation
            if (inst.flags == IROpFlags::None)
            {
                output << "         0,\n";
            }
            else
            {
                output << "         ";
                std::vector<std::string> flag_names;
                if ((inst.flags & IROpFlags::Parent) != IROpFlags::None)
                    flag_names.push_back("kIROpFlag_Parent");
                if ((inst.flags & IROpFlags::UseOther) != IROpFlags::None)
                    flag_names.push_back("kIROpFlag_UseOther");
                if ((inst.flags & IROpFlags::Hoistable) != IROpFlags::None)
                    flag_names.push_back("kIROpFlag_Hoistable");
                if ((inst.flags & IROpFlags::Global) != IROpFlags::None)
                    flag_names.push_back("kIROpFlag_Global");

                for (size_t i = 0; i < flag_names.size(); ++i)
                {
                    if (i > 0)
                        output << " | ";
                    output << flag_names[i];
                }
                output << ",\n";
            }

            output << "     }},\n";
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

        return output.str();
    }
};

int main(int argc, char* argv[])
{
    String input_file;
    String enum_output_file;
    String info_output_file;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

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
            input_file = arg.c_str();
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

        // Generate enum header
        IREnumGenerator generator;
        std::string enum_output = generator.generate(inst_set);

        // Write enum header file
        std::ofstream enum_file(enum_output_file.getBuffer());
        if (!enum_file.is_open())
        {
            std::cerr << "Error: Cannot create output file '" << enum_output_file << "'\n";
            return 1;
        }

        enum_file << enum_output;
        enum_file.close();

        std::cout << "Successfully generated " << enum_output_file << "\n";

        // Generate info table if requested
        if (info_output_file.getLength() > 0)
        {
            std::string info_output = generator.generateInfoTable(inst_set);

            std::ofstream info_file(info_output_file.getBuffer());
            if (!info_file.is_open())
            {
                std::cerr << "Error: Cannot create output file '" << info_output_file << "'\n";
                return 1;
            }

            info_file << info_output;
            info_file.close();

            std::cout << "Successfully generated " << info_output_file << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
