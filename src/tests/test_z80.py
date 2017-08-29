# -*- coding: utf-8 -*-

# $Id:$

import sys
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

    def test_load_memory(self):
        m = [randint(0, 255) for i in xrange(65536)]
        self.z.load_memory("".join([chr(i) for i in m]), 0)
        for i in xrange(65536):
            self.assertEqual(self.z.memory(i), m[i])


class TestZEXT(TestCase):

    def _test_file(self, name):

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

        with open(name, "rb") as f:
            data = f.read()
        z = Z80()
        z.load_memory(data, 0x100)
        z.memory(0, 0xd3)
        z.memory(1, 0x00)
        z.memory(5, 0xdb)
        z.memory(6, 0x00)
        z.memory(7, 0xc9)

        z.register("pc", 0x100)
        while not z.is_done:
            z.emulate(80000)

    def test_zexall(self):
        self._test_file("../z80emu/testfiles/zexall.com")

    def test_zexdoc(self):
        self._test_file("../z80emu/testfiles/zexdoc.com")
