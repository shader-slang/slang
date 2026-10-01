#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Adversarial physical report contracts; no compiler, toolkit or GPU is used."""
import collections
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import sys
from types import SimpleNamespace
import unittest

spec = importlib.util.spec_from_file_location('surfaces', Path(__file__).with_name('nvvm-surface-results.py'))
surfaces = importlib.util.module_from_spec(spec)
spec.loader.exec_module(surfaces)


class Contracts(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        real = surfaces.load_harness()
        self.rows = [r for r in real.cases() if r['case'] in ('half-written-nans', 'float32-1d-1-wholeStore')]
        self.h = SimpleNamespace(**dict(vars(real), cases=lambda: self.rows))
        self.tool = self.root / 'tool.bin'
        self.tool.write_bytes(b'fake executable identity; never executed')
        self.artifacts = self.h.freeze_identity([self.tool, Path(self.h.__file__)])
        self.h.inventory_paths = lambda args, provider: {Path(p) for p in self.artifacts}
        self.save(self.root / 'provenance.json', {'artifacts': self.artifacts})
        tool = surfaces.reference(self.tool)
        self.report = dict(status='passed', requested_cells=6, cells=[], identity_verification='unchanged',
                           provenance_sha256=surfaces.sha((self.root/'provenance.json').read_bytes()),
                           tool_sha256=surfaces.sha(Path(self.h.__file__).read_bytes()),
                           **{k: tool for k in ('slangc', 'provider', 'libnvvm', 'ptxas')})
        for row in self.rows:
            self.add_case(row)
        self.flush()

    def save(self, path, value):
        path.write_text(json.dumps(value, indent=2)+'\n')

    def add_case(self, row):
        folder = self.root / row['case']
        folder.mkdir()
        buffers = self.h.oracle(row)
        specs, data = self.h.resource_specs(row), self.h.resource_buffers(row, buffers)
        source = self.h.REPO/'tests/cuda'/('nvvm-surface-physical-'+row['fixture']+'.slang')
        files = self.h.oracle_files(row, buffers)
        for key, value in files.items():
            (folder/(key+'.bin')).write_bytes(value)
        self.save(folder/'oracle.json', dict(row=row,source_sha256=surfaces.sha(source.read_bytes()),
            file_sha256={k:surfaces.sha(v) for k,v in files.items()},
            converted_nan_positions=[sorted(x['nan_positions']) for x in data]))
        for mode in surfaces.MODES:
            d=folder/mode; d.mkdir()
            ptx='.target sm_80\n.entry '+row['entry']+'()\n.const .align 8 .b8 SLANG_globalParams['+str(8*len(specs))+']\n'
            (d/'code.ptx').write_text(ptx)
            (d/'code.cubin').write_bytes(b'CPU-only assembly artifact fixture')
            params=[]
            for i,s in enumerate(specs):
                ty={'kind':'scalar','scalarType':s['scalar']}
                if s['lanes']>1:ty={'kind':'vector','elementCount':s['lanes'],'elementType':ty}
                param={'name':s['name'],'binding':{'kind':'uniform','offset':8*i,'size':8},'type':{'kind':'resource','baseShape':f'texture{row["shape"]}D','access':'readWrite','resultType':ty}}
                if 'array_layers' in row:param['type']['array']=True
                if s['format']:param['format']=s['format']
                params.append(param)
            self.save(d/'reflection.json', {'parameters':params,'entryPoints':[{'name':row['entry'],'stage':'compute','threadGroupSize':[1,1,1]}]})
            runtime=dict(status='passed',initial_host_copies_verified=True,surface_bindings_verified=True,
                         global_surface_handles_uploaded=True,launched_and_synchronized=True,
                         actual_array_descriptors=[{'Width':row['width'],'Height':row['height'] if row['shape']>=2 else 0,'Depth':row.get('volume_depth',row.get('array_layers',0)),'Format':self.h.FORMATS[s['storage']][0],'NumChannels':s['lanes'],'Flags':3 if 'array_layers' in row else 2} for s in specs],
                         cleanup=[dict(operation=op,return_code=0) for op in ['cuModuleUnload']+['cuSurfObjectDestroy']*len(specs)+['cuArrayDestroy']*len(specs)+['cuCtxDestroy_v2']],readbacks=[])
            for i,(s,b) in enumerate(zip(specs,data)):
                (d/(s['name']+'-actual.bin')).write_bytes(b['expected'])
                checked=self.h.compare(row,buffers,i,b['expected'])
                checked.update(array=s['name'],active_texels=b['active_texels'],guard_texels=b['guard_texels'])
                runtime['readbacks'].append(checked)
            self.save(d/'runtime.json',runtime)
            cell=dict(case=row['case'],entry=row['entry'],mode=mode,source=str(source),source_sha256=surfaces.sha(source.read_bytes()),oracle_sha256=surfaces.sha((folder/'oracle.json').read_bytes()),status='passed',binding_verified=True,ptx_sha256=surfaces.sha((d/'code.ptx').read_bytes()),reflection_sha256=surfaces.sha((d/'reflection.json').read_bytes()),runtime=runtime)
            for phase,name in [('compile','compile'),('assemble','assemble'),('runtime_process','runtime')]:
                (d/(name+'.log')).write_text('')
                cell[phase]=dict(command=['fake-never-run'],return_code=0,log=str(d/(name+'.log')))
            command=[str(self.tool),str(source),'-target','ptx','-entry',row['entry'],'-stage','compute','-capability','cuda_sm_8_0','-O'+mode[-1],'-o',str(d/'code.ptx'),'-reflection-json',str(d/'reflection.json')]
            command += [f'-D{k}={v}' for k,v in row['defines'].items()]
            if mode.startswith('nvvm'):command += ['-emit-cuda-via-nvvm','-nvvm-path',str(self.tool)]
            cell['compile']['command']=command
            cell['assemble']['command']=[str(self.tool),'-arch=sm_80',str(d/'code.ptx'),'-o',str(d/'code.cubin')]
            self.save(d/'device-case.json',dict(row=row,ptx=str(d/'code.ptx'),oracle_sha256={k:surfaces.sha(v) for k,v in files.items()}))
            cell['runtime_process']['command']=[sys.executable,str(Path(self.h.__file__)),'--device-case',str(d/'device-case.json')]
            self.report['cells'].append(cell)

    def flush(self):
        self.report['counts']=dict(collections.Counter(c['status'] for c in self.report['cells']))
        self.report['status']='passed' if self.report['counts']=={'passed':len(self.report['cells'])} else 'failed'
        self.save(self.root/'results.json', self.report)

    def validate(self):
        self.flush()
        return surfaces.validate_report(self.root/'results.json', 0 if self.report['status']=='passed' else 1, self.h)

    def cell(self):
        return next(c for c in self.report['cells'] if c['case']=='half-written-nans')

    def runtime_edit(self, action):
        c=self.cell();action(c['runtime'])
        self.save(self.root/c['case']/c['mode']/'runtime.json',c['runtime'])

    def replace_actual(self, value, mismatch=False, channel=None):
        c=self.cell(); row=next(r for r in self.rows if r['case']==c['case'])
        buffers=self.h.oracle(row); data=self.h.resource_buffers(row,buffers)[0]
        path=self.root/c['case']/c['mode']/'surface-actual.bin'
        actual=bytearray(path.read_bytes())
        channel=next(iter(data['nan_positions'])) if channel is None else channel
        actual[channel*2:channel*2+2]=value.to_bytes(2,'little');path.write_bytes(actual)
        replay=self.h.compare(row,buffers,0,actual)
        replay.update(array='surface',active_texels=data['active_texels'],guard_texels=data['guard_texels'])
        c['runtime']['readbacks'][0]=replay
        if mismatch:
            c['status']=c['runtime']['status']='runtime-mismatch';c['runtime_process']['return_code']=1
        self.save(path.parent/'runtime.json',c['runtime'])

    def negative(self):
        c=self.cell();c['status']='compile-failed';c['compile']['return_code']=1
        Path(c['compile']['log']).write_text("error[E52017]: unsupported imageSubscript\n")
        for k in ('assemble','runtime_process','runtime'):del c[k]
        (self.root/c['case']/c['mode']/'runtime.json').unlink()

    def test_complete_report_bootstrap_and_preservation(self):
        block=self.validate()
        self.assertEqual(surfaces.compare(None,block)['status'],'review-required')
        self.assertEqual(surfaces.compare(block,block)['status'],'passed')

    def test_layered_and_volume_descriptors_and_array_role(self):
        cases = surfaces.load_harness().cases()
        layered = [r for r in cases if r['fixture'] == 'layered']
        volumes = [r for r in cases if r['fixture'] == 'half-volume']
        self.assertEqual({r['case'] for r in volumes}, {'half-3d-whole', 'half-3d-components'})
        for row in volumes:
            self.assertNotIn('array_layers', row)
            self.assertEqual((row['shape'], row['width'], row['height'], row['volume_depth']),
                             (3, 11, 5, 3))
        singleton = dict(next(r for r in layered if r['shape'] == 1),
                         case='native32-1d-array-single-layer', array_layers=1)
        spatial_rows = layered + [singleton] + volumes
        for row in spatial_rows:
            self.rows.append(row)
            self.add_case(row)
            self.report['requested_cells'] += 3
        block = self.validate()
        for outcome in block['fresh_cell_outcomes']:
            row = next(r for r in self.rows if r['case'] == outcome['id'])
            for resource in outcome['resources']:
                descriptor = resource['descriptor']
                self.assertEqual(descriptor['Height'], row['height'] if row['shape'] >= 2 else 0)
                self.assertEqual(descriptor['Depth'], row.get('volume_depth', row.get('array_layers', 0)))
                self.assertEqual(descriptor['Flags'], 3 if 'array_layers' in row else 2)

        for row in spatial_rows:
            cell = next(c for c in self.report['cells'] if c['case'] == row['case'])
            runtime_path = self.root / cell['case'] / cell['mode'] / 'runtime.json'
            descriptor = cell['runtime']['actual_array_descriptors'][0]
            depth = row.get('volume_depth', row.get('array_layers', 0))
            mutations = [('Depth', 0), ('Depth', depth + 1),
                         ('Flags', 2 if 'array_layers' in row else 3)]
            if row['shape'] >= 2:
                mutations.append(('Height', 0))
            for field, wrong in mutations:
                with self.subTest(case=row['case'], field=field, wrong=wrong):
                    correct = descriptor[field]
                    descriptor[field] = wrong
                    self.save(runtime_path, cell['runtime'])
                    with self.assertRaisesRegex(ValueError, 'wrong actual resource descriptors'):
                        self.validate()
                    descriptor[field] = correct
                    self.save(runtime_path, cell['runtime'])

        # Rehash the modified reflection so rejection proves the array role, not file tampering.
        for row in spatial_rows + [self.rows[0]]:
            cell = next(c for c in self.report['cells'] if c['case'] == row['case'])
            reflection_path = self.root / cell['case'] / cell['mode'] / 'reflection.json'
            reflection = surfaces.read(reflection_path)
            original = reflection_path.read_bytes()
            reflection['parameters'][0]['type']['array'] = 'array_layers' not in row
            self.save(reflection_path, reflection)
            cell['reflection_sha256'] = surfaces.sha(reflection_path.read_bytes())
            with self.subTest(case=row['case']):
                with self.assertRaisesRegex(ValueError, 'Surface array role mismatch'):
                    self.validate()
            reflection_path.write_bytes(original)
            cell['reflection_sha256'] = surfaces.sha(original)

    def test_inventory_faults(self):
        original=copy.deepcopy(self.report)
        for fault in ('missing','duplicate','whole-case','requested','mode','case'):
            with self.subTest(fault=fault):
                self.report=copy.deepcopy(original)
                if fault=='missing':self.report['cells'].pop()
                elif fault=='duplicate':self.report['cells'].append(copy.deepcopy(self.report['cells'][0]))
                elif fault=='whole-case':self.report['cells']=[c for c in self.report['cells'] if c['case']!='half-written-nans']
                elif fault=='requested':self.report['requested_cells']=3
                else:self.report['cells'][0][fault]='unknown'
                with self.assertRaises(ValueError):self.validate()

    def test_process_faults(self):
        original=copy.deepcopy(self.report)
        for phase in ('compile','assemble','runtime_process'):
            for field,value in [('return_code',None),('return_code',-11),('return_code',124),('timed_out',True),('error','failed to spawn'),('return_code',1)]:
                with self.subTest(phase=phase,field=field,value=value):
                    self.report=copy.deepcopy(original);self.cell()[phase][field]=value
                    with self.assertRaises(ValueError):self.validate()

    def test_runtime_proof_faults(self):
        original=copy.deepcopy(self.report)
        actions={
            'launch':lambda r:r.pop('launched_and_synchronized'),
            'upload':lambda r:r.pop('initial_host_copies_verified'),
            'binding':lambda r:r.pop('surface_bindings_verified'),
            'handles':lambda r:r.pop('global_surface_handles_uploaded'),
            'missing-readback':lambda r:r['readbacks'].pop(),
            'duplicate-readback':lambda r:r['readbacks'].append(copy.deepcopy(r['readbacks'][0])),
            'descriptor':lambda r:r['actual_array_descriptors'][0].update(Format=0x20),
            'cleanup':lambda r:r['cleanup'][0].update(return_code=1),
            'missing-cleanup':lambda r:r['cleanup'].pop(),
        }
        for name,action in actions.items():
            with self.subTest(fault=name):
                self.report=copy.deepcopy(original);self.runtime_edit(action)
                with self.assertRaises(ValueError):self.validate()

    def test_missing_runtime_false_pass(self):
        self.cell().pop('runtime')
        with self.assertRaises((ValueError,KeyError)):self.validate()

    def test_wrong_success_phase_and_changed_diagnostic(self):
        before=self.validate()
        self.cell()['status']='assemble-failed'
        with self.assertRaises(ValueError):self.validate()
        self.cell()['status']='passed';Path(self.cell()['compile']['log']).write_text('new warning\n')
        self.assertEqual(surfaces.compare(before,self.validate())['status'],'review-required')

    def test_source_oracle_or_artifact_tampering(self):
        original=copy.deepcopy(self.report)
        for field in ('source_sha256','oracle_sha256','ptx_sha256','reflection_sha256'):
            with self.subTest(field=field):
                self.report=copy.deepcopy(original);self.cell()[field]='wrong'
                with self.assertRaises(ValueError):self.validate()
        self.report=original;self.report['identity_verification']='failed'
        with self.assertRaises(ValueError):self.validate()
        self.report['identity_verification']='unchanged';self.tool.write_bytes(b'changed')
        with self.assertRaises(ValueError):self.validate()

    def test_missing_identity_artifact_and_raw_status_tampering(self):
        self.flush()
        report=copy.deepcopy(self.report);report['status']='failed'
        self.save(self.root/'results.json',report)
        with self.assertRaises(ValueError):surfaces.validate_report(self.root/'results.json',1,self.h)
        provenance=surfaces.read(self.root/'provenance.json')
        del provenance['artifacts'][str(Path(self.h.__file__))]
        self.save(self.root/'provenance.json',provenance)
        self.report['provenance_sha256']=surfaces.sha((self.root/'provenance.json').read_bytes())
        with self.assertRaises(ValueError):self.validate()

    def test_oracle_binary_tampering(self):
        path=self.root/self.cell()['case']/'expected.bin';path.write_bytes(b'tampered')
        with self.assertRaises(ValueError):self.validate()

    def test_nan_and_guard_false_pass_independently(self):
        original_report = copy.deepcopy(self.report)
        path = self.root/self.cell()['case']/self.cell()['mode']/'surface-actual.bin'
        original_bytes = path.read_bytes()
        for channel in (0,15):
            with self.subTest(channel=channel):
                self.report = copy.deepcopy(original_report)
                path.write_bytes(original_bytes)
                # Slot0 is converted NaN; slot15 is untouched. Restore all bytes/proof each time.
                self.replace_actual(0x1234,channel=channel)
                with self.assertRaises(ValueError):self.validate()

    def test_finite_active_byte_and_assembly_output_tampering(self):
        c=self.cell();folder=self.root/c['case']/c['mode']
        actual=folder/'observed-actual.bin';data=bytearray(actual.read_bytes());data[0]^=1;actual.write_bytes(data)
        with self.assertRaises(ValueError):self.validate()
        data[0]^=1;actual.write_bytes(data);(folder/'code.cubin').write_bytes(b'')
        with self.assertRaises(ValueError):self.validate()

    def test_allowed_nan_payload_change_preserves_outcome(self):
        before=self.validate();self.replace_actual(0xfe55)
        self.assertEqual(surfaces.compare(before,self.validate())['status'],'passed')

    def test_nan_to_infinity_and_changed_mismatch_signature(self):
        self.replace_actual(0x7c00,mismatch=True);before=self.validate()
        self.replace_actual(0xfc00,mismatch=True);after=self.validate()
        self.assertEqual(before['fresh_cell_outcomes'][3]['status'],after['fresh_cell_outcomes'][3]['status'])
        self.assertEqual(surfaces.compare(before,after)['status'],'review-required')

    def test_unchanged_compile_negative_and_diagnostic_transition(self):
        self.negative();before=self.validate()
        self.assertEqual(surfaces.compare(before,self.validate())['status'],'passed')
        Path(self.cell()['compile']['log']).write_text('error[E100]: provider missing\n')
        self.assertEqual(surfaces.compare(before,self.validate())['status'],'review-required')

    def test_negative_becomes_pass_requires_review(self):
        positive=self.validate();self.negative();negative=self.validate()
        self.assertEqual(surfaces.compare(negative,positive)['status'],'review-required')

    def test_new_complete_negative_requires_review_and_incomplete_rejected(self):
        before=self.validate()
        new=next(r for r in self.h.cases() if r['case']=='half-written-nans').copy()
        new['case']='new-negative';self.rows.append(new);self.add_case(new);self.report['requested_cells']+=3
        for c in self.report['cells'][-3:]:
            c['status']='compile-failed';c['compile']['return_code']=1;Path(c['compile']['log']).write_text('error[E52017]: unsupported\n')
            for k in ('assemble','runtime_process','runtime'):del c[k]
            (self.root/c['case']/c['mode']/'runtime.json').unlink()
        after=self.validate();self.assertEqual(surfaces.compare(before,after)['status'],'review-required')
        self.report['cells'].pop()
        with self.assertRaises(ValueError):self.validate()

    def test_slang255_negative_is_valid_but_crash_timeout_is_not(self):
        self.negative();self.cell()['compile']['return_code']=255
        self.assertEqual(self.validate()['status'],'validated')
        for value in (None,-11,256):
            self.cell()['compile']['return_code']=value
            with self.assertRaises(ValueError):self.validate()

    def test_compile_mode_and_backend_contract(self):
        c=next(c for c in self.report['cells'] if c['mode']=='nvvm-o3')
        original=list(c['compile']['command'])
        c['compile']['command']=[x.replace('-O3','-O0') for x in original]
        with self.assertRaises(ValueError):self.validate()
        c['compile']['command']=[x for x in original if x!='-emit-cuda-via-nvvm']
        with self.assertRaises(ValueError):self.validate()

    def test_comparator_source_and_oracle_changes_require_review(self):
        before=self.validate()
        for key in ('source_sha256','case_sha256','oracle_sha256'):
            after=copy.deepcopy(before);after['fresh_cell_outcomes'][0][key]='changed'
            self.assertEqual(surfaces.compare(before,after)['status'],'review-required')

    def test_library_alias_accepts_resolved_reference_and_rejects_redirection(self):
        target = self.root / 'libnvvm.so.4.0.0'
        target.write_bytes(b'fake library; never loaded')
        alias = self.root / 'libnvvm.so'
        alias.symlink_to(target.name)
        # Inventory the selected symlink, while the report and compiler name its resolved target.
        self.artifacts.update(self.h.freeze_identity([alias]))
        self.assertNotIn(str(target), self.artifacts)
        self.report['libnvvm'] = surfaces.reference(target)
        for cell in self.report['cells']:
            command = cell['compile']['command']
            if '-nvvm-path' in command:
                command[command.index('-nvvm-path') + 1] = str(target)
        self.save(self.root / 'provenance.json', {'artifacts': self.artifacts})
        self.report['provenance_sha256'] = surfaces.sha((self.root / 'provenance.json').read_bytes())
        self.assertEqual(self.validate()['status'], 'validated')
        # Equal bytes cannot excuse changing the selected library target after the snapshot.
        other = self.root / 'redirected.so'
        other.write_bytes(target.read_bytes())
        alias.unlink()
        alias.symlink_to(other.name)
        with self.assertRaises(ValueError):
            self.validate()

    def test_compiler_identity_is_provenance_not_outcome_equality(self):
        before=self.validate();after=copy.deepcopy(before);after['identity']['slangc']['sha256']='new compiler'
        self.assertEqual(surfaces.compare(before,after)['status'],'passed')


if __name__=='__main__':
    unittest.main()
