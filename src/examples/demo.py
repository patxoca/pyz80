#!/usr/bin/env python
# -*- coding: utf-8 -*-

from pyz80 import Z80

Z80_CPU_SPEED = 4000000  # In Hz.
CYCLES_PER_STEP = int(Z80_CPU_SPEED / 50)


def demo():
    print "Initializing"

    z = Z80()

    program = [
        # ORG 0x4000
        0x3e, 0x42,        # LD A, 0x42
        0x11, 0x12, 0x40,  # LD DE, DST
        0x01, 0x00, 0x10,  # LD BC, 0x1000
        0x12,              # LD (DE), A
        0x0b,              # DEC BC
        0x62,              # LD H, D
        0x6b,              # LD L, E
        0x13,              # INC DE
        0xed, 0xb0,        # LDIR
        0xcd, 0x00, 0x00   # CALL 0
        # DST: área de datos a inicializar, dirección 0x4012
    ]

    z.reset()

    # Inicializar llamada 0: finaliza emulador
    z.memory(0, 0xd3)      # OUT (0), A
    z.memory(1, 0x00)

    # Cargar el programa
    for i, v in enumerate(program):
        z.memory(0x4000 + i, v)

    # Inicializar el PROGRAM COUNTER
    z.register("pc", 0x4000)

    while not z.is_done:
        z.emulate(CYCLES_PER_STEP)

    # Restamos los ciclos consumidor por CALL 0 + OUT 0, A
    print "%i cycle(s)\n" % (z.cycles - 28);


if __name__ == "__main__":
    demo()
