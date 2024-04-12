import re
import optparse
import os
import subprocess
import gspread
from oauth2client.service_account import ServiceAccountCredentials

scope = [
        'https://spreadsheets.google.com/feeds',
        'https://www.googleapis.com/auth/spreadsheets',
        'https://www.googleapis.com/auth/drive',
        'https://www.googleapis.com/auth/drive.file'
        ]

creds_str = os.environ[ 'slang_verif_svc' ]

creds_dict = eval(creds_str)

creds = ServiceAccountCredentials.from_json_keyfile_dict(creds_dict, scope)
client = gspread.authorize(creds)

raw_data_sheet = client.open("VKCTS newly passing").sheet1
raw_data_records = raw_data_sheet.get_all_records()
raw_data_index = len(raw_data_records) + 2
clear_range = raw_data_sheet.range("A1:A" + str(raw_data_index))
for i in range(0, raw_data_index)
    clear_range[i].value = ""
raw_data_sheet.update_cells(clear_range)

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
newly_passing = []
for item in passing:
    if not item in list_passing:
        if first:
            print("\nNew passing test(s)")
            first = False
        print(item)
        newly_passing.append(item)


if len(newly_passing) > 0:
    cell_list = raw_data_sheet.range("A1:A" + str(len(newly_passing) + 1))
    i = 0

    for new_passing in newly_passing:
        cell_list[i].value = new_passing
        i += 1
      
    raw_data_sheet.update_cells(cell_list)



