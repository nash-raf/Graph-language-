#!/usr/bin/env python3
"""Check benchmark kernel instructions and record per-worker TLS footprint."""
import re
import subprocess
from tdg_joint_build import BUILD, OUT


def instructions(executable, function):
    assembly = subprocess.check_output(['objdump', '-d', '--no-show-raw-insn',
                                      f'--disassemble={function}', str(executable)], text=True)
    result = []
    for line in assembly.splitlines():
        match = re.match(r'\s*[0-9a-f]+:\s+(.*)', line)
        if not match:
            continue
        instruction = match[1].split('#')[0].strip()
        instruction = re.sub(r'\b[0-9a-f]+ (?=<)', '', instruction)
        instruction = re.sub(r'-?0x[0-9a-f]+(?=\(%rip\))', 'RELOCATION', instruction)
        instruction = re.sub(r'%fs:0x[0-9a-f]+', '%fs:TLS_OFFSET', instruction)
        result.append(instruction)
    assert result, function
    return result


def main():
    policies = ['reference', 'joint', 'previous']
    lines = []
    for function in ('body', 'range', 'task'):
        code = [instructions(BUILD/f'probe_{p}', function) for p in policies]
        assert code[0] == code[1] == code[2], function
        lines.append(f'PASS {function}: same {len(code[0])} instructions after address/TLS relocation normalization')
    for policy in policies:
        elf = subprocess.check_output(['readelf', '-lW', str(BUILD/f'probe_{policy}')], text=True)
        row = next(line.split() for line in elf.splitlines() if line.strip().startswith('TLS '))
        lines.append(f'{policy}: TLS={int(row[5],16)} bytes')
    (OUT/'kernel_check.txt').write_text('\n'.join(lines)+'\n')
    print('\n'.join(lines))


if __name__ == '__main__':
    main()
