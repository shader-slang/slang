import re
import optparse


passing = []
list_passing = []


summary = "Test run totals:\n"

useage = "useage: %prog [options]"
parser = optparse.OptionParser( useage )

parser.add_option( "-p", "--passing_list", dest="PASSINGLIST", help="The test list of expected passing vk-gl-cts tests [default: %default]", default="slang-passing-tests.txt" )
parser.add_option( "-r", "--vk_cts_results", dest="RESULTS", help="The output from running vk-gl-cts [default: %default]", default="vk-gl-cts-results.txt" )

(options, args) = parser.parse_args()

test_found = False

with open(options.RESULTS, 'r') as tl_f:
    for test_line in tl_f:
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
            summary += test_line
        elif test_line.startswith("  Failed:"):
            summary += test_line
        elif test_line.startswith("  Not supported:"):
            summary += test_line
        elif test_line.startswith("  Warnings:"):
            summary += test_line
        elif test_line.startswith("  Waived:"):
            summary += test_line


with open(options.PASSINGLIST, 'r') as tl_f:
    for test_line in tl_f:
        test_line = test_line.rstrip()
        list_passing.append(test_line)

for thing in passing:
    if not thing in list_passing:
        print(thing)

print(summary)

