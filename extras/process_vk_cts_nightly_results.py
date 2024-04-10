import re
import optparse
import os
import subprocess

proc_env = os.environ.copy()
proc_env["DISABLE_CTS_SLANG"] = "0"

passing = []
list_passing = []


summary = "Test run totals:\n"

useage = "useage: %prog [options]"
parser = optparse.OptionParser( useage )

parser.add_option( "-p", "--passinglist", dest="PASSINGLIST", help="The test list of expected passing vk-gl-cts tests [default: %default]", default="slang-passing-tests.txt" )
parser.add_option( "-a", "--archive_dir", dest="ARCHIVE_DIR", help="The directory where the vulkan directory is found [default: %default]", default="." )
parser.add_option( "-f", "--testlist", dest="TESTLIST", help="The test list used to run vk-gl-cts [default: %default]", default="all-tests.txt" )

(options, args) = parser.parse_args()

cmd = ".\deqp-vk.exe --deqp-archive-dir=" + options.ARCHIVE_DIR + " --deqp-caselist-file=" + options.TESTLIST
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, text=True,
                           env=proc_env)
stdout, stderr = proc.communicate()

test_found = False


for test_line in stdout.splitlines():
    match = re.search(r'Test case \'(.*)\'', test_line)
    if match:
        test_name = match.group(1)
        test_found = True
    elif test_found:
        result_match = re.search(r'\s*Pass \(All sub\-cases passed\)', test_line)
        if result_match:
            test_found = False
            passing.append(test_name)

    if test_line.startswith("  Passed:"):
        summary += test_line + "\n"
    elif test_line.startswith("  Failed:"):
        summary += test_line + "\n"
    elif test_line.startswith("  Not supported:"):
        summary += test_line + "\n"
    elif test_line.startswith("  Warnings:"):
        summary += test_line + "\n"
    elif test_line.startswith("  Waived:"):
        summary += test_line + "\n"

print(summary)

with open(options.PASSINGLIST, 'r') as tl_f:
    for test_line in tl_f:
        test_line = test_line.rstrip()
        list_passing.append(test_line)

first = True
for item in passing:
    if not item in list_passing:
        if first:
            print("\nNew passing test(s)")
            first = False
        print(item)



