#!/usr/bin/env python3

import os
import re
import sys

# globals
verbose = False

def verbosePrint(s):
    global verbose

    if verbose:
        print(s)

def verbosePrintNoNewline(s):
    global verbose

    if verbose:
        print(s, end="")


def printHelpAndExit():
    print('''Scans markdown files and checks for broken relative links

Usage: check-markdown-relative-links.py [options] <files>

Options:
-v      Verbose output
''')
    sys.exit(1)


def scanForAnchor(file, anchorMatcher, filename, anchor):
    for line in file:
        for m in anchorMatcher.finditer(line):
            verbosePrint(f"  - found anchor {m.group(1)}")
            if anchor == m.group(1):
                return

    raise NameError(f"Anchor '{anchor}' not found in file '{filename}'")

def checkMarkDownLinks(srcFile):
    errors = 0
    linkMatcher = re.compile(r"\[(?:[^]\\]|\.)*\]\(([^)#]*)([^)]*)\)")
    anchorMatcher = re.compile(r"^#.*\{(#[^}]+)\}")

    verbosePrint(f"Collecting links: {srcFile}")

    with open(srcFile) as file1:
        lineNo = 0
        for line in file1:
            lineNo = lineNo + 1
            for m in linkMatcher.finditer(line):
                linkDstFile = m.group(1)
                linkDstAnchor = m.group(2)
                verbosePrintNoNewline(f"- {linkDstFile} {linkDstAnchor}:")
                if linkDstFile.startswith("https://"):
                    verbosePrint("URL (skipping check)");
                    continue

                if len(linkDstFile) == 0:
                    dstFile = srcFile
                else:
                    dstFile = os.path.join(os.path.dirname(srcFile), linkDstFile)

                try:
                    with open(dstFile) as file2:
                        if len(linkDstAnchor) > 0:
                            verbosePrint("")
                            scanForAnchor(file2, anchorMatcher, dstFile, linkDstAnchor)
                except FileNotFoundError:
                    errors = errors + 1
                    print(f"{srcFile}:{lineNo}: Link destination file {dstFile} not found!")
                    continue

                except NameError:
                    errors = errors + 1
                    print(f"{srcFile}:{lineNo}: Link destination file {dstFile} does not define anchor {linkDstAnchor}")
                    continue

                verbosePrint("OK")

    return errors

def main(argv):
    global verbose

    # version check -- this script was developed with Python 3.12
    if sys.version_info < (3, 12):
        warning(f"Python version {sys.version_info.major}.{sys.version_info.minor} is not at least 3.12!")

    # parse options
    while len(argv) > 0:
        if argv[0] == '-v':
            verbose = True
            argv = argv[1:]
            continue

        break

    if len(argv) == 0:
        printHelpAndExit();

    errors = 0
    for f in argv:
        errors += checkMarkDownLinks(f)

    print(f"Encountered {errors} errors")

if __name__ == "__main__":
    main(sys.argv[1:])
