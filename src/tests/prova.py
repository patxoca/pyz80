from pyz80 import Z80

Z80_CPU_SPEED = 4000000  # In Hz.
CYCLES_PER_STEP = int(Z80_CPU_SPEED / 50)


print "Initializing"

z = Z80()

program = [
    0x3e, 0x42, 0x11, 0x12, 0x40, 0x01, 0x00, 0x10,
    0x12, 0x0b, 0x62, 0x6b, 0x13, 0xed, 0xb0, 0xcd,
    0x00, 0x00
]
z.reset()

# Put instruction OUT 0, A into address 0x0000.
z.poke(0, 0xd3)
z.poke(1, 0x00)

# Load program into memory
for i, v in enumerate(program):
    z.poke(0x4000 + i, v)

# Set program counter
z.load_register("PC", 0x4000)

total = 0;
while not z.is_done:
    z.emulate(CYCLES_PER_STEP);

# subtract the cycles required by the CALL 0 + OUT 0, A/
# print "%i cycle(s)\n" % (total - 28);


print "Checking memory content ...",
i = 0x4012
n = 0x1000
while n:
    if z.peek(i) != 0x42:
        raise ValueError(i)
    i += 1
    n -= 1
print "ok"
