// main.cpp
//
// Lightweight standalone tool that prints slangc command-line help in the
// requested style (default: markdown).  Built without heavy compiler deps
// so it compiles fast in CI.

#include "../core/slang-command-options-writer.h"
#include "../core/slang-command-options.h"
#include "../slang/slang-options.h"

#include <cstdio>
#include <cstring>

using namespace Slang;

int main(int argc, char** argv)
{
    CommandOptionsWriter::Style style = CommandOptionsWriter::Style::Markdown;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-help-style") == 0 && i + 1 < argc)
        {
            ++i;
            if (strcmp(argv[i], "text") == 0)
                style = CommandOptionsWriter::Style::Text;
            else if (strcmp(argv[i], "markdown") == 0)
                style = CommandOptionsWriter::Style::Markdown;
            else if (strcmp(argv[i], "no-link-markdown") == 0)
                style = CommandOptionsWriter::Style::NoLinkMarkdown;
            // ignore unknown style values
        }
    }

    CommandOptions cmdOptions;
    initCommandOptions(cmdOptions);

    CommandOptionsWriter::Options writerOptions;
    writerOptions.style = style;
    auto writer = CommandOptionsWriter::create(writerOptions);
    auto& buf = writer->getBuilder();

    if (style == CommandOptionsWriter::Style::Text)
    {
        buf << "Usage:\n";
        buf << "  slangc [options...] [--] <input files>\n\n";
    }
    else
    {
        buf << "# Slang Command Line Options\n\n";
        buf << "*Usage:*\n";
        buf << "```\n";
        buf << "slangc [options...] [--] <input files>\n\n";
        buf << "# For help\n";
        buf << "slangc -h\n\n";
        buf << "# To generate this file\n";
        buf << "slangc -help-style markdown -h\n";
        buf << "```\n";
    }

    writer->appendDescription(&cmdOptions);

    fwrite(buf.getBuffer(), 1, (size_t)buf.getLength(), stdout);

    return 0;
}
