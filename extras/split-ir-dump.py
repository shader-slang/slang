#!/usr/bin/env python3
"""
Split Slang IR dump files into separate sections for easier analysis.

Usage:
    python split-ir-dump.py <dump-file>
    python split-ir-dump.py < dump-file
    slangc.exe -dump-ir -target spirv-asm test.slang | python split-ir-dump.py

Examples:
    # From file
    build/Debug/bin/slangc.exe -dump-ir -target spirv-asm test.slang > test.dump
    python split-ir-dump.py test.dump

    # From stdin (pipe)
    build/Debug/bin/slangc.exe -dump-ir -target spirv-asm test.slang | python split-ir-dump.py

    # From stdin (redirect)
    python split-ir-dump.py < test.dump
"""

import sys
import os
import re
from pathlib import Path


def find_next_dump_dir():
    """Find the next available dump-XXX directory."""
    idx = 0
    while True:
        dir_name = f"dump-{idx:03d}"
        if not os.path.exists(dir_name):
            return dir_name
        idx += 1


def sanitize_section_name(section_name):
    """Convert section name to a safe filename."""
    # Remove the colon at the end
    name = section_name.rstrip(':').strip()

    # Replace spaces and special characters with hyphens
    name = re.sub(r'[^\w\-]', '-', name)

    # Remove consecutive hyphens
    name = re.sub(r'-+', '-', name)

    # Remove leading/trailing hyphens
    name = name.strip('-')

    return name


def split_dump(content, source_name="stdin"):
    """Split IR dump content into separate section files.

    Args:
        content: The IR dump content as a string
        source_name: Name of the source (for display purposes)
    """
    # Create output directory
    output_dir = find_next_dump_dir()
    os.makedirs(output_dir)

    # Split into sections based on ### headers
    lines = content.split('\n')

    sections = []
    current_section = None
    current_content = []

    for line in lines:
        if line.startswith('### ') and ':' in line:
            # Start of a new section
            if current_section is not None:
                sections.append((current_section, '\n'.join(current_content)))

            current_section = line[4:]  # Remove "### " prefix
            current_content = []
        elif line.strip() == '###':
            # End of current section (delimiter)
            if current_section is not None:
                sections.append((current_section, '\n'.join(current_content)))
                current_section = None
                current_content = []
        else:
            # Content line
            if current_section is not None:
                current_content.append(line)

    # Handle any remaining content
    if current_section is not None and current_content:
        sections.append((current_section, '\n'.join(current_content)))

    # Write sections to separate files
    for idx, (section_name, section_content) in enumerate(sections, start=1):
        safe_name = sanitize_section_name(section_name)
        filename = f"{idx:03d}-{safe_name}.txt"
        filepath = os.path.join(output_dir, filename)

        with open(filepath, 'w', encoding='utf-8') as f:
            # Write header
            f.write(f"# Section: {section_name}\n")
            f.write(f"# Original section number: {idx}\n")
            f.write("#" + "=" * 70 + "\n\n")

            # Write content
            f.write(section_content)

            # Add trailing newline if missing
            if not section_content.endswith('\n'):
                f.write('\n')

    # Create an index file
    index_file = os.path.join(output_dir, "000-INDEX.txt")
    with open(index_file, 'w', encoding='utf-8') as f:
        f.write("IR Dump Section Index\n")
        f.write("=" * 70 + "\n")
        f.write(f"Source: {source_name}\n")
        f.write(f"Total sections: {len(sections)}\n")
        f.write("\n")
        f.write("Sections:\n")
        f.write("-" * 70 + "\n")

        for idx, (section_name, section_content) in enumerate(sections, start=1):
            safe_name = sanitize_section_name(section_name)
            filename = f"{idx:03d}-{safe_name}.txt"
            line_count = section_content.count('\n')
            f.write(f"{idx:3d}. {filename:50s} ({line_count:5d} lines)\n")

    print(f"Results saved to: {output_dir}/")

    return 0


def main():
    # Check if we're reading from stdin or a file
    if len(sys.argv) == 1:
        # No arguments - read from stdin
        if sys.stdin.isatty():
            # stdin is a terminal, not a pipe - show usage
            print("Usage: python split-ir-dump.py <dump-file>")
            print("       python split-ir-dump.py < dump-file")
            print("       slangc.exe -dump-ir ... | python split-ir-dump.py")
            print("\nExamples:")
            print("  # From file")
            print("  build/Debug/bin/slangc.exe -dump-ir -target spirv-asm -o tmp.spv test.slang > test.dump")
            print("  python split-ir-dump.py test.dump")
            print()
            print("  # From stdin (pipe) - RECOMMENDED")
            print("  build/Debug/bin/slangc.exe -dump-ir -target spirv-asm -o tmp.spv test.slang | python split-ir-dump.py")
            print()
            print("  # From stdin (redirect)")
            print("  python split-ir-dump.py < test.dump")
            print()
            print("Note: Use -o <file> to prevent compiled output from mixing with IR dump")
            return 1

        # Reading from stdin
        content = sys.stdin.read()
        return split_dump(content, source_name="stdin")

    elif len(sys.argv) == 2:
        # Reading from file
        dump_file = sys.argv[1]
        if not os.path.exists(dump_file):
            print(f"Error: Dump file '{dump_file}' not found")
            return 1

        with open(dump_file, 'r', encoding='utf-8') as f:
            content = f.read()

        return split_dump(content, source_name=dump_file)

    else:
        print("Error: Too many arguments")
        print("\nUsage: python split-ir-dump.py <dump-file>")
        print("       python split-ir-dump.py < dump-file")
        print("       slangc.exe -dump-ir -o <file> ... | python split-ir-dump.py")
        return 1


if __name__ == '__main__':
    sys.exit(main())
