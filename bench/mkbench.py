#!/usr/bin/python3

import sys, re, textwrap

# takes hex string, returns byte array
def from_hex(s):
    r = re.compile(r'[^0-9a-fA-F]')
    hex_only = r.sub('', s)
    chars = [int(c, 16) for c in textwrap.wrap(hex_only, 2)]
    return bytearray(chars)

if len(sys.argv) < 3:
    print('Usage: ./mkbench.py <opcode> <ofile> [prelude=\'\'] [iters=200]')
    print('Opcode and prelude should be in hex. If provided, prelude will be first in output ROM.')
    exit()

opcode = sys.argv[1]
ofile = sys.argv[2]
prelude = ''
iters = 1500
if len(sys.argv) >= 4:
    prelude = sys.argv[3]

if len(sys.argv) >= 5:
    iters = int(sys.argv[4])

ba = from_hex(prelude)
opcode = from_hex(opcode)

for i in range(iters):
    ba += opcode

# append exit instruction
ba += bytearray([0x00, 0xFD])

with open(ofile, 'wb') as fd:
    fd.write(ba)
