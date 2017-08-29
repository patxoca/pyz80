Bindings python de un emulador del procesador Z80.

Esto es una prueba de concepto, todo muy pre-alfa.

El emulador está escrito en C (https://github.com/anotherlin/z80emu)
con lo que la velocidad de python no es un problema.

Python por su parte proporciona la facilidad de programación y la
posibilidad de experimentar con el emulador desde el intérprete
interactivo.

El binding permite asociar al emulador una función (escrita en python)
que será llamada en ciertos momentos. Esto permite simular hardware
adicional o llamadas al sistema sin necesidad de escribir C. En el
ejemplo `zexall.py <./src/examples/zexall.py>`_ se muestra como
utilizar esto para simular llamadas al sistema CP/M y poder ejecutar
en el emulador un binario compilado para ese sistema.


Tutorial
========

Crear un emulador:

.. code-block:: python

   from pyz80 import Z80

   emu = Z80()


Leer la memoria, un byte:

.. code-block:: python

   emu.memory(0x1234)


Escribir la memoria, un byte:

.. code-block:: python

   emu.memory(0x1234, 0xAB)


Escribir un bloque de memoria, n bytes:

.. code-block:: python

   data = "\0x01\0x02\0x03"
   emu.load_memory(data, 0x1234)


Leer un registro:

.. code-block:: python

   emu.register("A")


Escribir un registro:

.. code-block:: python

   emu.register("A", 0xAB)


Asignar *syscall*:

.. code-block:: python

   def my_syscall(z):
       print "SYSCALL"

   emu.syscall(my_syscall)


Comprobar si la emulación ha finalizado:

.. code-block:: python

   if emu.is_done:
       print "Fin"


Obtener el número de ciclos emulados:

.. code-block:: python

   print emu.cycles


Ejemplos
========

El directorio `src/examples <./src/examples>`_ contiene varios
ejemplos:

- `demo.py <./src/examples/demo.py>`_: ejemplo sencillo en el que se
  muestra como cargar un programa y ejecutarlo.

- `zexall.py <./src/examples/zexall.py>`_: muestra como implementar
  llamadas al sistema.


TODO
====

- Mejorar syscall. Actualmente es la adaptación directa del programa
  de ejemplo que viene con el emulador. En ese sentido me da que es
  una solución especializada que podria mejorarse, pero antes necesito
  aprender mas del Z80.

- Mejorar la API, no es muy *pythonica*.

- En ``load_memory`` utilizar un ``bytearray`` en lugar de un ``str``.

- Añadir *profiling*. Por el momento añadir contadores para las
  lecturas i escrituras de la RAM.
