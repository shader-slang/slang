# InstTrace Debugging Utility
#
# This script is used to trace the callstack at the creation of a specific IR instruction in the Slang compiler.
# This is useful for debugging purposes, especially if you encounter a compiler bug related to an instruction
# that appears to be incorrect, and you want to find out where it was created in the codebase.
#
# Usage: python3 ./extras/insttrace.py <inst UID> <commandline_to_slangc_or_slang-test>
#
# The script will print the callstack at the point where the specified instruction was created.
# <inst UID> is the unique identifier of the instruction you want to trace, which can be found by inspecting
# the _debugUID field of an IRInst object in the Slang compiler source code.
# If the instruction is a clone of another instruction, it will also trace the creation of the original instruction
# recursively.

import sys
import subprocess
import re
import os

def traceInst(inst_uid, command):
    # Run the command with the provided arguments
    # Set the environment variable SLANG_IR_ALLOC_BREAK to the instruction UID
    env = dict(os.environ, SLANG_DEBUG_IR_BREAK=inst_uid)
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    stdout, stderr = process.communicate()
    print(f"Inst #{inst_uid} is created at: ")

    # Parse the output to find the string between "BEGIN IR Trace" and "END IR Trace"
    ir_trace = re.search(r"BEGIN IR Trace(.*?)END IR Trace", stdout.decode(encoding='utf-8', errors='ignore'), re.DOTALL)
    if not ir_trace:
        print("No IR Trace found in the output.")
        return

    traceOutput = ir_trace.group(1)
    regex = r"(\S+)\(\+(0x[0-9a-f]+)\) \[0x[0-9a-f]+\]"

    lines = traceOutput.splitlines()
    # First we collect all addresses to call addr2line to convert them to file and line numbers.
    addresses = []
    for line in lines:
        # Match lines that look like "/home/yongh/slang/build/Debug/bin/../lib/libslang.so(+0xe0e11a) [0x7f95f8e0e11a]"
        # and convert the address to a file and line number using addr2line
        match = re.search(regex, line)
        if match:
            libFile = match.group(1)
            address = match.group(2)
            addresses.append((libFile, address))

    for line in lines:
        # Match lines that look like "/home/yongh/slang/build/Debug/bin/../lib/libslang.so(+0xe0e11a) [0x7f95f8e0e11a]"
        # and convert the address to a file and line number using addr2line
        # The regex should capture the library file name and the address.
        match = re.search(regex, line)
        
        if match:
            libFile = match.group(1)
            address = match.group(2)
            # Use addr2line to convert the address to a file and line number
            addr2line_command = ["addr2line", "-e", libFile, "-f", "-C", address]
            addr2line_process = subprocess.Popen(addr2line_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = addr2line_process.communicate()
            outputStr = stdout.decode(encoding='utf-8', errors='ignore').replace("\n", " ").strip()
            print(outputStr)
    else:
        # print the line as is if it doesn't match the address format
        print(line)

    print("(end of stacktrace)")

    # Find "Inst #%u is a clone of Inst #%u" in the trace output, and trace the original instruction
    # if it exists.
    clone_match = re.search(r"Inst #(\d+) is a clone of Inst #(\d+)", traceOutput)
    if clone_match:
        clone_inst_uid = clone_match.group(1)
        original_inst_uid = clone_match.group(2)

        print(f"Note: Inst #{clone_inst_uid} is a clone of Inst #{original_inst_uid}.")
        traceInst(original_inst_uid, command)

def main():
    if len(sys.argv) < 3:
        print("InstTrace Debugging Utility")
        print("Usage: python insttrace.py <inst UID> <commandline_to_slangc_or_slang-test>")
        sys.exit(1)

    inst_uid = sys.argv[1]
    command = sys.argv[2:]
    traceInst(inst_uid, command)

if __name__ == "__main__":
    main()