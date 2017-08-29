# -*- coding: utf-8 -*-

# $Id:$

from random import randint
from unittest import TestCase
from pyz80 import Z80


class _Base(TestCase):
    reg8 = [
        "A", "B", "C", "D", "E", "F", "H", "L",
        "IXH", "HX", "IXL", "LX", "IYH", "HY", "IYL", "LY",
    ]

    reg16 = [
        "BC", "DE", "HL", "AF",
        "IX", "IY", "SP", "PC",
    ]

    registers = reg8 + reg16


class TestZ80(_Base):

    def setUp(self):
        self.z = Z80()

    def test_reset(self):
        self.z.reset()

        for i in xrange(0x10000):
            self.assertEqual(self.z.memory(i), 0)

        for r in self.registers:
            rv = self.z.register(r)
            if r in ("A", "F"):
                self.assertEqual(rv, 0xFF)
            elif r in ("AF", "SP"):
                self.assertEqual(rv, 0xFFFF)
            else:
                self.assertEqual(rv, 0)

    def test_memory_write(self):
        for i in xrange(0x10000):
            v = randint(0, 255)
            self.z.memory(i, v)
            self.assertEqual(self.z.memory(i), v)

    def test_register_write(self):
        for r in self.reg8:
            v = randint(0, 255)
            self.z.register(r, v)
            self.assertEqual(self.z.register(r), v)

        for r in self.reg16:
            v = randint(0, 65535)
            self.z.register(r, v)
            self.assertEqual(self.z.register(r), v)

    def test_load_memory_from_str(self):
        m = [randint(0, 255) for i in xrange(65536)]
        self.z.load_memory("".join([chr(i) for i in m]), 0)
         for i in xrange(65536):
             self.assertEqual(self.z.memory(i), m[i])

    def test_load_memory_from_bytearray(self):
        m = bytearray([randint(0, 255) for i in xrange(65536)])
        self.z.load_memory(m, 0)
        for i in xrange(65536):
            self.assertEqual(self.z.memory(i), m[i])


class TestZEXT(TestCase):

    def _test_file(self, name):

        # Ejecuta el binario 'name' capturando la salida por pantalla
        # (solo funciones 2 i 9) en la lista 'stdout'. Una vez
        # finalizada la emulación comprueba la salida para detectar
        # errores.
        #
        # La primera línea de la salida es una cabecera:
        #
        #   Z80 instruction exerciser
        #
        # A continuación vienen los tests. Un test que pasa se muestra
        # como:
        #
        #   <adc,sbc> hl,<bc,de,hl,sp>... OK
        #
        # i uno que falla como:
        #
        #   <adc,sbc> hl,<bc,de,hl,sp>...
        #   CRC:xxxxxxxx expected:xxxxxxxx
        #
        # La última línea de la salida es un pie:
        #
        #   Tests complete
        #

        stdout = []
        wrt = stdout.append

        def syscall(z):
            if z.register("c") == 2:
                wrt(chr(z.register("e")))
                return

            if z.register("c") == 9:
                address = z.register("DE")
                count = 0
                while address < 65536 and z.memory(address) != ord("$") and count < 100:
                    wrt(chr(z.memory(address)))
                    address += 1
                    count += 1
                return

        with open(name, "rb") as f:
            data = f.read()
        z = Z80()
        z.syscall(syscall)
        z.load_memory(data, 0x100)
        z.memory(0, 0xd3)
        z.memory(1, 0x00)
        z.memory(5, 0xdb)
        z.memory(6, 0x00)
        z.memory(7, 0xc9)

        z.register("pc", 0x100)
        while not z.is_done:
            z.emulate(80000)

        lines = ("".join(stdout)).split("\n\r")
        for line in lines[1:-1]:
            self.assertEqual(line[-2:], "OK", line)


    def test_zexall(self):
        self._test_file("../z80emu/testfiles/zexall.com")

    def test_zexdoc(self):
        self._test_file("../z80emu/testfiles/zexdoc.com")
