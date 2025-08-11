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

    # Parse the output to find the string between "BEGIN IR Trace" and "END IR Trace"
    ir_trace = re.search(r"BEGIN IR Trace(.*?)END IR Trace", stdout.decode(encoding='utf-8', errors='ignore'), re.DOTALL)
    if not ir_trace:
        print("No IR Trace found in the output.")
        return

    traceOutput = ir_trace.group(1)
    regex = r"(\S+)\(\+(0x[0-9a-f]+)\) \[0x[0-9a-f]+\]"

    lines = traceOutput.splitlines()
    
    # First, collect all addresses grouped by library file for batching
    lib_addresses = {}  # libFile -> list of addresses
    line_info = []  # (line, libFile, address) for each line
    
    for line in lines:
        match = re.search(regex, line)
        if match:
            libFile = match.group(1)
            address = match.group(2)
            line_info.append((line, libFile, address))
            
            if libFile not in lib_addresses:
                lib_addresses[libFile] = []
            lib_addresses[libFile].append(address)
        else:
            line_info.append((line, None, None))
    
    # Batch call addr2line for each library file
    address_to_symbol = {}  # (libFile, address) -> symbol info
    
    for libFile, addresses in lib_addresses.items():
        if not addresses:
            continue
            
        # Call addr2line once with all addresses for this library
        addr2line_command = ["addr2line", "-e", libFile, "-f", "-C"] + addresses
        try:
            addr2line_process = subprocess.Popen(addr2line_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = addr2line_process.communicate()
            
            # Parse the output - addr2line returns function name and location for each address
            output_lines = stdout.decode(encoding='utf-8', errors='ignore').strip().split('\n')
            
            # Each address produces 2 lines: function name, then file:line
            for i, address in enumerate(addresses):
                if i * 2 + 1 < len(output_lines):
                    function_name = output_lines[i * 2].strip()
                    location = output_lines[i * 2 + 1].strip()
                    symbol_info = f"{function_name} {location}"
                    address_to_symbol[(libFile, address)] = symbol_info
                else:
                    address_to_symbol[(libFile, address)] = f"<unknown> {address}"
        except Exception as e:
            # Fallback if addr2line fails
            for address in addresses:
                address_to_symbol[(libFile, address)] = f"<addr2line failed> {address}"
    
    # Now print the results using the cached symbol information
    for line, libFile, address in line_info:
        if libFile and address:
            symbol_info = address_to_symbol.get((libFile, address), f"<not found> {address}")
            print(symbol_info)
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