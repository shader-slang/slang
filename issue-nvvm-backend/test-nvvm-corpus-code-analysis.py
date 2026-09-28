#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""CPU contracts for static PTX/SASS inventories and explicitly limited normalization."""
import hashlib
import importlib.util
from pathlib import Path
import struct
import unittest

REPO=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('analysis',REPO/'extras/analyze-nvvm-corpus-code.py')
a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
PTX='''
.version 8.7
.target sm_80
.address_size 64
.file 1 "path//contains.func fake(){}"
.extern .func (.param .b32 retval) external(.param .b32 input);
.visible .entry computeMain(.param .u64 computeMain_param_0)
{
 .reg .pred %p<2>;
 .reg .b32 %r<4>;
 .reg .b64 %rd<2>;
 .loc 1 2 0
 ld.param.u64 %rd1, [computeMain_param_0];
 mov.u32 %r1, %tid.x;
 setp.eq.u32 %p1, %r1, 0;
 @!%p1 bra $done;
 { .reg .b32 %tmp;
   call.uni ( %r2 ), helper, ( %r1 );
 }
$done:
 st.global.u32 [%rd1], %r2;
 ret;
}
.func (.param .b32 result) helper(.param .b32 value)
{
 .reg .b32 %r<3>;
 ld.param.u32 %r1, [value];
 add.u32 %r2, %r1, 7;
 st.param.b32 [result], %r2;
 ret;
}
.func unused(){ ret; }
'''


class AnalysisContracts(unittest.TestCase):
    def test_instruction_and_reachable_inventory(self):
        d=a.parse_ptx(PTX,'computeMain')
        self.assertEqual(d['entry_metrics']['instructions'],7)
        self.assertEqual(d['reachable_metrics']['instructions'],11)
        self.assertEqual(d['reachable_functions'],['computeMain','helper'])
        self.assertEqual(d['module_functions'],3)
        self.assertEqual(d['entry_metrics']['virtual_registers'],{'pred':2,'b32':5,'b64':2})
        self.assertEqual(d['entry_metrics']['memory']['ld_param'],1)
        self.assertEqual(d['entry_metrics']['memory']['st_global'],1)
        self.assertEqual(d['entry_metrics']['branches'],1)

    def test_comments_and_strings_do_not_create_instructions(self):
        altered=PTX.replace('mov.u32','/* add.u32 %r1,1,2; */ mov.u32')+'\n// .entry fake() { ret; }'
        self.assertEqual(a.parse_ptx(PTX,'computeMain'),a.parse_ptx(altered,'computeMain'))

    def test_alpha_renaming_preserves_generated_symbol_identity(self):
        altered=PTX.replace('computeMain_param_0','argument').replace('helper','generated_helper').replace('$done','$renamed').replace('%r','%renamed')
        self.assertEqual(a.parse_ptx(PTX,'computeMain')['normalized_stream_sha256'],a.parse_ptx(altered,'computeMain')['normalized_stream_sha256'])

    def test_constant_predicate_and_operand_changes_are_not_equal(self):
        expected=a.parse_ptx(PTX,'computeMain')['normalized_stream_sha256']
        for old,new in [('7;','8;'),('@!%p1','@%p1'),('%r2, %r1, 7','%r2, %r2, 7'),('st.global','st.shared')]:
            with self.subTest(change=new):
                self.assertNotEqual(expected,a.parse_ptx(PTX.replace(old,new),'computeMain')['normalized_stream_sha256'])

    def test_architectural_special_register_is_not_renamed(self):
        self.assertNotEqual(a.parse_ptx(PTX,'computeMain')['normalized_stream_sha256'],a.parse_ptx(PTX.replace('%tid.x','%ctaid.x'),'computeMain')['normalized_stream_sha256'])

    def test_vector_operands_and_multiline_instruction_count_once(self):
        text='.entry main(){ .reg .b32 %r<4>; ld.global.v2.u32\n { %r1, %r2 }, [ %r3 ]; ret; }'
        self.assertEqual(a.parse_ptx(text,'main')['entry_metrics']['instructions'],2)

    def test_unresolved_call_remains_explicit(self):
        text=PTX.replace('helper, (','external, (')
        d=a.parse_ptx(text,'computeMain')
        self.assertEqual(len(d['unresolved_calls']),1)
        self.assertEqual(d['reachable_metrics']['instructions'],7)

    def test_duplicate_and_unterminated_functions_rejected(self):
        for text in [PTX+'\n.entry computeMain(){ ret; }', '.entry main(){ mov.u32 %r1, 1 }']:
            with self.assertRaises(ValueError):a.parse_ptx(text,'computeMain' if 'computeMain' in text else 'main')

    def test_sass_counts_include_padding_but_exclude_encoding_lines(self):
        text='''code for sm_89
Function : main
/*0000*/ @!P0 BRA 0x20; /* 0x12345678 */
                       /* 0x000fe00000000000 */
/*0010*/ NOP; /* 0x55555555 */
/*0020*/ EXIT; /* 0x55555555 */
'''
        d=a.parse_sass(text)['main'];self.assertEqual(d['instructions'],3);self.assertEqual(d['non_nop_instructions'],2)
        with self.assertRaises(ValueError):a.parse_sass('cuobjdump fatal: nvdisasm unavailable')

    def test_bare_registers_and_scoped_redeclarations(self):
        text='.entry main(){ .reg .pred pred; .reg .b32 temp; mov.u32 temp, 1; @pred ret; { .reg .b32 temp; mov.u32 temp, 2; } ret; }'
        d=a.parse_ptx(text,'main')
        self.assertEqual(d['entry_metrics']['virtual_registers'],{'pred':1,'b32':2})
        self.assertIsNone(d['normalized_stream_sha256'])
        self.assertEqual(d['entry_metrics']['instructions'],4)

    def test_label_targets_and_duplicate_labels(self):
        text='.entry main(){ .reg .pred %p; @%p bra first; first: mov.u32 %r, 1; last: ret; }'
        self.assertNotEqual(a.parse_ptx(text,'main')['normalized_stream_sha256'],a.parse_ptx(text.replace('bra first','bra last'),'main')['normalized_stream_sha256'])
        with self.assertRaises(ValueError):a.parse_ptx(text.replace('last:','first:'),'main')

    def test_braces_inside_pragma_strings(self):
        text='.entry main(){ .pragma "ignore a closing } brace"; ret; }'
        self.assertEqual(a.parse_ptx(text,'main')['entry_metrics']['instructions'],1)

    def test_sass_malformed_lines_offsets_and_coverage(self):
        for body in ['/*0000*/ UNKNOWN WITHOUT SEMICOLON\n/*0010*/ EXIT;', '/*0010*/ EXIT;', '/*0000*/ NOP;\n/*0000*/ EXIT;']:
            with self.assertRaises(ValueError):a.parse_sass('Function : main\n'+body)
        sass=a.parse_sass('Function : main\n/*0000*/ EXIT;')
        a.validate_sass_coverage(sass,{'.text.main':{'bytes':16}})
        with self.assertRaises(ValueError):a.validate_sass_coverage(sass,{'.text.main':{'bytes':32}})

    def test_artifact_case_and_mode_binding(self):
        declared=dict(id='one',mode='nvvm-o3',corpus='frozen',source='one.slang',
                      generated='/capture/one.slang',dump='/capture/one',accepted_outcome={'classification':'correct'})
        cell={**declared,'ptx':{'path':'/capture/one/code.ptx'},'actual_output':{'path':'/capture/one.slang.actual.txt'}}
        a.validate_cell_binding(cell,declared)
        for key,value in [('mode','nvrtc-o3'),('ptx',{'path':'/capture/two/code.ptx'}),('actual_output',{'path':'/capture/two.slang.actual.txt'})]:
            with self.assertRaises(ValueError):a.validate_cell_binding({**cell,key:value},declared)

    def test_actual_elf_section_bytes(self):
        names=b'\0.shstrtab\0.text.main\0';code=b'0123456789abcdef';offset=64;table=offset+len(names)+len(code)
        header=struct.pack('<16sHHIQQQIHHHHHH',b'\x7fELF\x02\x01'+b'\0'*10,2,190,1,0,0,table,0,64,0,0,64,3,1)
        empty=bytes(64);strings=struct.pack('<IIQQQQIIQQ',1,3,0,0,offset,len(names),0,0,1,0)
        section=struct.pack('<IIQQQQIIQQ',11,1,6,0,offset+len(names),len(code),0,0,16,0)
        d=a.executable_sections(header+names+code+empty+strings+section)
        self.assertEqual(d,{'.text.main':{'bytes':16,'sha256':hashlib.sha256(code).hexdigest()}})
        with self.assertRaises(ValueError):a.executable_sections(b'not an ELF file')


if __name__=='__main__':unittest.main()
