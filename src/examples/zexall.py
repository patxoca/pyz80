#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Este ejemplo muestra como definir nuestra propia función syscall
# implementada en python.
#
# Para el ejemplo se ejecuta el programa ZEXALL.COM. Este programa se
# utiliza para comprobar el funcionamiento de los emuladores de Z80.
# ZEXALL utiliza tres llamadas al sistema:
#
# - CALL 0: finalizar la emulación.
#
# - CALL 5, C=2: muestra por pantalla el carácter almacenado en el
#   registro E.
#
# - CALL 5, C=9: muestra por pantalla la cadena apuntada por el
#   registro DE. El final de la cadena debe estar marcado con el
#   caràcter '$' (dolar). Por precaución, la longitud de la cadena
#   está limitada a 100 carácteres.
#
# Estas són llamadas al sistema CP/M, sobre el que desarrolló
# originalmente el ZEXALL.
#
# La primera llamada está implementada en el propio emulador. Las
# otras dos estan implementadas en la función syscall, mas abajo.
#
# Las llamadas al sistema se implementan mediante una tabla de
# dispatch ubicada entre las direcciones 0 i 0x100. En este ejemplo
# únicamente las entradas 0 i 5 contienen opcodes IN i OUT, que són
# interceptado por el emulador y producen una llamada a nuestra
# función syscall (el IN).

import sys

from pyz80 import Z80

Z80_CPU_SPEED = 4000000  # In Hz.
CYCLES_PER_STEP = int(Z80_CPU_SPEED / 50)


def syscall(z):
    if z.register("c") == 2:
        sys.stdout.write(chr(z.register("e")))
        return

    if z.register("c") == 9:
        address = z.register("DE")
        count = 0
        while address < 65536 and z.memory(address) != ord("$") and count < 100:
            sys.stdout.write(chr(z.memory(address)))
            address += 1
            count += 1
        return


def test_binary(name):

    with open(name, "rb") as f:
        data = f.read()

    # Inicializar el emulador
    z = Z80()

    # Definir la función syscall
    z.syscall(syscall)

    # cargar el programa en memoria
    z.load_memory(data, 0x100)

    # Inicializar la tabla de dispatch

    # syscall 0
    z.memory(0, 0xd3)    # OUT (0), A
    z.memory(1, 0x00)

    # syscall 5
    z.memory(5, 0xdb)    # IN A, (0)
    z.memory(6, 0x00)
    z.memory(7, 0xc9)    # RET

    # Definir el PROGRAM COUNTER
    z.register("pc", 0x100)

    # Ejeutar emulación
    while not z.is_done:
        z.emulate(CYCLES_PER_STEP)


if __name__ == "__main__":
    test_binary("zexall.com")
