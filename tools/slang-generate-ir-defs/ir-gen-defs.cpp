#include "ir-yaml-parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

class IRDefsGenerator
{
private:
    int current_indent = 0;
    std::ostringstream output;

    void indent()
    {
        for (int i = 0; i < current_indent; ++i)
        {
            output << "    ";
        }
    }

    std::string flagsToString(IROpFlags flags)
    {
        if (flags == IROpFlags::None)
            return "0";

        std::vector<std::string> flag_names;
        if ((flags & IROpFlags::Parent) != IROpFlags::None)
            flag_names.push_back("PARENT");
        if ((flags & IROpFlags::UseOther) != IROpFlags::None)
            flag_names.push_back("USE_OTHER");
        if ((flags & IROpFlags::Hoistable) != IROpFlags::None)
            flag_names.push_back("HOISTABLE");
        if ((flags & IROpFlags::Global) != IROpFlags::None)
            flag_names.push_back("GLOBAL");

        std::string result;
        for (size_t i = 0; i < flag_names.size(); ++i)
        {
            if (i > 0)
                result += " | ";
            result += flag_names[i];
        }
        return result;
    }

    void generateEntry(const Entry* entry, IROpFlags inherited_flags)
    {
        if (entry->getType() == Entry::INSTRUCTION)
        {
            const auto* inst = static_cast<const InstructionEntry*>(entry);
            IROpFlags combined_flags = inst->flags | inherited_flags;

            // Print comment if present
            if (!inst->comment.empty())
            {
                // Handle multi-line comments
                std::istringstream comment_stream(inst->comment);
                std::string line;
                bool first_line = true;
                while (std::getline(comment_stream, line))
                {
                    indent();
                    if (first_line)
                    {
                        output << "// " << line << "\n";
                        first_line = false;
                    }
                    else
                    {
                        output << "// " << line << "\n";
                    }
                }
            }

            indent();
            output << "INST(" << inst->type_name << ", " << inst->mnemonic << ", " << inst->operands
                   << ", " << flagsToString(combined_flags) << ")\n";
        }
        else
        {
            const auto* range = static_cast<const RangeEntry*>(entry);

            // Print range comment if present
            if (!range->comment.empty())
            {
                // Handle multi-line comments
                std::istringstream comment_stream(range->comment);
                std::string line;
                while (std::getline(comment_stream, line))
                {
                    indent();
                    output << "// " << line << "\n";
                }
            }

            // Add comment for range name
            indent();
            output << "/* " << range->name << " */\n";

            // Increase indent for range contents
            current_indent++;

            // Generate nested entries
            IROpFlags combined_flags = range->flags | inherited_flags;
            for (const auto& nested : range->insts)
            {
                generateEntry(nested.get(), combined_flags);
            }

            // Decrease indent
            current_indent--;

            // Generate INST_RANGE
            const auto* first = range->findFirstInstruction();
            const auto* last = range->findLastInstruction();

            if (first && last)
            {
                indent();
                output << "INST_RANGE(" << range->name << ", " << first->type_name << ", "
                       << last->type_name << ")\n";
            }
        }
    }


public:
    std::string generate(const InstructionSet& inst_set)
    {
        // Generate prelude
        output << R"(// slang-ir-inst-defs.h

#ifndef INST
#error Must #define `INST` before including `ir-inst-defs.h`
#endif

#ifndef INST_RANGE
#define INST_RANGE(BASE, FIRST, LAST) /* empty */
#endif

#define PARENT kIROpFlag_Parent
#define USE_OTHER kIROpFlag_UseOther
#define HOISTABLE kIROpFlag_Hoistable
#define GLOBAL kIROpFlag_Global

)";

        // Generate instructions
        for (const auto& entry : inst_set.insts)
        {
            generateEntry(entry.get(), IROpFlags::None);
            output << "\n";
        }

        output << R"(#undef PARENT
#undef USE_OTHER
#undef INST_RANGE
#undef INST
)";

        return output.str();
    }
};

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input.yaml> <output.h>\n";
        return 1;
    }

    try
    {
        // Open input file
        std::ifstream input_file(argv[1]);
        if (!input_file.is_open())
        {
            std::cerr << "Error: Cannot open input file '" << argv[1] << "'\n";
            return 1;
        }

        // Parse YAML
        YAMLInstructionParser parser;
        InstructionSet inst_set = parser.parse(input_file);
        input_file.close();

        // Generate output
        IRDefsGenerator generator;
        std::string output = generator.generate(inst_set);

        // Write output file
        std::ofstream output_file(argv[2]);
        if (!output_file.is_open())
        {
            std::cerr << "Error: Cannot create output file '" << argv[2] << "'\n";
            return 1;
        }

        output_file << output;
        output_file.close();

        std::cout << "Successfully generated " << argv[2] << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
