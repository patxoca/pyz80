Bindings python de un emulador del procesador Z80.

Esto es una prueba de concepto, todo muy pre-alfa.

El emulador está escrito en C (https://github.com/anotherlin/z80emu)
con lo que la velocidad no es un problema.

Python por su parte proporciona la facilidad de programación y la
posibilidad de hacer pruebas interactivas desde el interprete.

Ejemplo:

.. code-block:: python
   :linenos:

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

   while not z.is_done:
       z.emulate(CYCLES_PER_STEP);
