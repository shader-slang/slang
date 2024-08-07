import os
import shutil
import glob
import subprocess
import argparse
import halo
import sys
import prettytable
import json

from timeit import timeit

### Setup ###

def clear_mkdir(dir):
    if os.path.exists(dir):
        shutil.rmtree(dir)
    os.makedirs(dir, exist_ok=True)

clear_mkdir('modules')
clear_mkdir('targets')
clear_mkdir('targets/generated')

target_choices = ['spirv', 'spirv-glsl', 'dxil', 'dxil-embedded']

parser = argparse.ArgumentParser()
parser.add_argument('--target', type=str, default='spirv', choices=target_choices)
parser.add_argument('--samples', type=int, default=1)
parser.add_argument('--output', type=str, default='benchmarks.json')

args = parser.parse_args(sys.argv[1:])

if not os.path.exists('slang-benchmarks'):
    repo = 'ssh://git@gitlab-master.nvidia.com:12051/slang/slang-benchmarks.git'
    command = f'git clone {repo}'
    subprocess.check_output(command)

dxc = 'dxc.exe'
slangc = '..\\..\\build\\Release\\bin\\slangc.exe'
target = args.target
samples = args.samples

if target == 'spirv':
    target = 'spirv -emit-spirv-directly'
    target_ext = 'spirv'
    embed = False
elif target == 'spirv-glsl':
    target = 'spirv -emit-spirv-via-glsl'
    target_ext = 'spirv'
    embed = False
elif target == 'dxil-embedded':
    target_ext = 'dxil'
    embed = True
elif target == 'dxil':
    target_ext = 'dxil'
    embed = False

print(f'slangc:  {slangc}')
print(f'target:  {target}')
print(f'samples: {samples}\n')

### Utility ###

def parse(results):
    results = results.split('\n')
    results = [ r for r in results if r.startswith('[*]') ]
    results = [ r.split() for r in results ]
    profile = {}
    for r in results:
        profile[r[1]] = float(r[-1][:-2])
    return profile

timings = {}
def run(command, key):
    profile = {}
    for i in range(samples):
        try:
            results = subprocess.check_output(command, stderr=subprocess.STDOUT, shell=True).decode('utf-8')
        except subprocess.CalledProcessError as exc:
            print(exc.output.decode('utf-8'))
            return
            # exit(-1)

        p = parse(results)
        if len(profile) == 0:
            profile = p
        else:
            for k, v in p.items():
                profile[k] += v

    for k in profile:
        profile[k] /= samples

    timings[key] = profile

def compile_cmd(file, output, stage=None, entry=None, emit=False):
    cmd = f'{slangc} -report-perf-benchmark {file}'

    if stage:
        cmd += f' -stage {stage}'
        if entry:
            cmd += f' -entry {entry}'
        else:
            cmd += f' -entry {stage}'

    if emit:
        cmd += f' -target {target_ext}'
        output += '.' + target_ext
    elif embed:
        cmd += ' -embed-dxil'
        cmd += ' -profile lib_6_6'
        cmd += ' -incomplete-library'

    cmd += f' -o {output}'

    return cmd

### Module precompilation ###

modules = []

with halo.Halo(text=' compiling modules...', spinner='dots') as spinner:
    for file in glob.glob('slang-benchmarks\\mdl\\*.slang'):
        if file.endswith('hit.slang'):
            run(compile_cmd(file, 'modules/closesthit.slang-module', stage='closesthit'), 'module/closesthit')
            run(compile_cmd(file, 'modules/anyhit.slang-module', stage='anyhit'), 'module/anyhit')
            run(compile_cmd(file, 'modules/shadow.slang-module', stage='anyhit', entry='shadow'), 'module/shadow')
        else:
            basename = os.path.basename(file)
            run(compile_cmd(file, f'modules/{basename}-module'), 'module/' + file)
            modules.append(f'modules/{basename}-module')

        spinner.info(f' compiled {file}.'), spinner.start()

### Entrypoint compilation ###
with halo.Halo(text=' compiling entrypoints...', spinner='dots') as spinner:
    hit = 'slang-benchmarks/mdl/hit.slang'
    files = ' '.join(modules)

    # Module
    cmd = compile_cmd(f'{files} modules/closesthit.slang-module', f'targets/dxr-ch-modules', stage='closesthit', emit=True)
    run(cmd, f'full/{target_ext}/module/closesthit')
    
    spinner.info(f'compiled closesthit (module)'), spinner.start()

    cmd = compile_cmd(f'{files} modules/anyhit.slang-module', f'targets/dxr-ah-modules', stage='anyhit', emit=True)
    run(cmd, f'full/{target_ext}/module/anyhit')
    
    spinner.info(f'compiled anyhit (module)'), spinner.start()

    cmd = compile_cmd(f'{files} modules/shadow.slang-module', f'targets/dxr-sh-modules', stage='anyhit', entry='shadow', emit=True)
    run(cmd, f'full/{target_ext}/module/shadow')

    spinner.info(f'compiled shadow (module)'), spinner.start()

    # Monolithic    
    cmd = compile_cmd(hit, f'targets/dxr-ch-mono', stage='closesthit', emit=True)
    run(cmd, f'full/{target_ext}/mono/closesthit')
    
    spinner.info(f'compiled shadow (monolithic)'), spinner.start()

    cmd = compile_cmd(hit, f'targets/dxr-ah-mono', stage='anyhit', emit=True)
    run(cmd, f'full/{target_ext}/mono/anyhit')
    
    spinner.info(f'compiled shadow (monolithic)'), spinner.start()

    cmd = compile_cmd(hit, f'targets/dxr-sh-mono', stage='anyhit', entry='shadow', emit=True)
    run(cmd, f'full/{target_ext}/mono/shadow')
    
    spinner.info(f'compiled shadow (monolithic)'), spinner.start()

# Module precompilation time
precompilation_time = 0
for k in timings:
    if k.startswith('module'):
        precompilation_time += timings[k]['compileInner']

timings[f'full/{target_ext}/precompilation'] = { 'compileInner': precompilation_time }

# Output to benchmark file
json_data = []
for k, v in timings.items():
    if not k.startswith('full'):
        continue

    name = k.split('/')[1:]
    name = ' : '.join(reversed(name))

    data = {
        'name': name,
        'unit': 'milliseconds',
        'value': v['compileInner']
    }

    json_data.append(data)

# TODO: append target to benchmark file name
with open(args.output, 'w') as file:
    json.dump(json_data, file, indent=4)

# Generate readable Markdown as well
print(4 * '\n')
print('# Slang MDL benchmark results\n')
print('## Module precompilation time\n')
print(f'Total: **{timings[f'full/{target_ext}/precompilation']['compileInner']} ms**\n')

print('## Module compilation for entry points\n')

entries = [ 'Closest Hit', 'Any Hit', 'Shadow' ]
prefixes = [ 'closesthit', 'anyhit', 'shadow' ]

table = prettytable.PrettyTable()
table.set_style(prettytable.MARKDOWN)
table.field_names = [ 'Entry', 'Total' ]

total = 0
for entry, prefix in zip(entries, prefixes):
    row = [ entry ]
    db = timings[f'full/{target_ext}/module/{prefix}']
    spCompile = db['compileInner']
    row.append(f'{spCompile:.3f}s')
    table.add_row(row)
    total += spCompile

print(f'Total: **{total} ms**\n')
print(table, end='\n\n')

print('## Monolithic compilation for entry points\n')

table = prettytable.PrettyTable()
table.set_style(prettytable.MARKDOWN)
table.field_names = [ 'Entry', 'Total' ]

total = 0
for entry, prefix in zip(entries, prefixes):
    row = [ entry ]
    db = timings[f'full/{target_ext}/mono/{prefix}']
    spCompile = db['compileInner']
    row.append(f'{spCompile:.3f}s')
    table.add_row(row)
    total += spCompile

print(f'Total: **{total} ms**\n')
print(table, end='\n\n')