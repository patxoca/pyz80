#include <Python.h>
#include <structmember.h>

#include "z80emu/z80emu.h"
#include "zext.h"

#undef DEBUG

#ifdef DEBUG
#define TRACE(format, ...) printf("DEBUG: " format, ##__VA_ARGS__)
#else
#define TRACE(format, ...)
#endif

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    Z80_STATE state;
    TContext  context;
} PyZ80;


typedef struct {
    char *name;
    int dw;
    int index;
} TRegisterMapping;


static const TRegisterMapping register_mapping[] = {
    {"A", 0, Z80_A},
    {"B", 0, Z80_B},
    {"C", 0, Z80_C},
    {"D", 0, Z80_D},
    {"E", 0, Z80_E},
    {"F", 0, Z80_F},
    {"H", 0, Z80_H},
    {"L", 0, Z80_L},

    {"IXH", 0, Z80_IXH},
    {"HX",  0, Z80_IXH},
    {"IXL", 0, Z80_IXL},
    {"LX",  0, Z80_IXL},
    {"IYH", 0, Z80_IYH},
    {"HY",  0, Z80_IYH},
    {"IYL", 0, Z80_IYL},
    {"LY",  0, Z80_IYL},

    {"BC", 1, Z80_BC},
    {"DE", 1, Z80_DE},
    {"HL", 1, Z80_HL},
    {"AF", 1, Z80_AF},

    {"IX", 1, Z80_IX},
    {"IY", 1, Z80_IY},
    {"SP", 1, Z80_SP},

    {"PC", 2, 0},

    {NULL}
};

/* lookup_register(regname)
 *
 * Dado un nombre de registro lo busca en la tabla register_mapping.
 * Si corresponde a un registro válido devuelve un puntero a la
 * entrada correspondiente, eoc devuelve NULL.
 *
 * La búsqueda es insensible a las mayúsculas/minúsculas.
 */
static const TRegisterMapping *lookup_register(const char *regname) {
    const TRegisterMapping *p = register_mapping;

    while (p->name != NULL) {
        if (strcasecmp(p->name, regname) == 0) {
            return p;
        }
        p++;
    }
    return NULL;
}


static void Z80_dealloc(PyZ80* self) {
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *Z80_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyZ80 *self;

    TRACE("new\n");

    self = (PyZ80 *)type->tp_alloc(type, 0);
    if (self != NULL) {
        Z80Reset(&(self->state));
        memset(&(self->context.memory), 0, MEMORY_SIZE);
        self->context.is_done = 0;
    }

    return (PyObject *)self;
}

static int Z80_init(PyZ80 *self, PyObject *args, PyObject *kwds) {
    // NOTE: __init__ will/may receive the initial value for the
    // registers, and the memory.

    TRACE("init\n");

    /* PyObject *first=NULL, *last=NULL, *tmp; */

    /* static char *kwlist[] = {"first", "last", "number", NULL}; */

    /* if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist, */
    /*                                   &first, &last, */
    /*                                   &self->number)) */
    /*     return -1; */

    /* if (first) { */
    /*     tmp = self->first; */
    /*     Py_INCREF(first); */
    /*     self->first = first; */
    /*     Py_XDECREF(tmp); */
    /* } */

    /* if (last) { */
    /*     tmp = self->last; */
    /*     Py_INCREF(last); */
    /*     self->last = last; */
    /*     Py_XDECREF(tmp); */
    /* } */

    return 0;
}

static PyObject *Z80_reset(PyZ80 *self) {
    TRACE("reseting the Z80!\n");
    Z80Reset(&(self->state));
    memset(&(self->context.memory), 0, MEMORY_SIZE);
    self->context.is_done = 0;

    Py_RETURN_NONE;
}

static PyObject *Z80_interrupt(PyZ80 *self, PyObject *args, PyObject *kwargs) {
    int data;

    static char *kwlist[] = {"data", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &data))
        return NULL;

    TRACE("interrupt data %i\n", data);
    Z80Interrupt(&(self->state), data, &(self->context));
    Py_RETURN_NONE;
}

static PyObject *Z80_non_maskable_interrupt(PyZ80 *self) {
    TRACE("NMI\n");
    Z80NonMaskableInterrupt(&(self->state), &(self->context));
    Py_RETURN_NONE;
}

static PyObject *Z80_emulate(PyZ80 *self, PyObject *args, PyObject *kwargs) {
    int num_cycles;

    static char *kwlist[] = {"cycles", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &num_cycles))
        return NULL;

    TRACE("emulate %i cycles\n", num_cycles);
    Z80Emulate(&(self->state), num_cycles, &(self->context));
    Py_RETURN_NONE;
}

static PyObject *Z80_getmemory(PyZ80 *self, void *closure)
{
    PyObject *memory;

    // TODO: create and return a buffer (or a memoryview, not sure).
    // The object is for external consumption so I'd expect that it
    // would'nt be INCREFed here ... or maybe yes ... a new buffer for
    // each call or just one shared by all calls?
    //
    // https://docs.python.org/2/c-api/buffer.html
    memory = PyString_FromString("[memory placeholder]");

    return memory;
}

static PyObject *Z80_get_isdone(PyZ80 *self, void *closure)
{
    if (self->context.is_done) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/*
 * FIXME: until I see how the API will work I'll define some methods to
 * update the state. Less fashionable but easier to implement.
 */

/*
 * Load memory block.
 *
 *   load_memory(data, address)
 *
 * data: string
 * address: integer (0-65536)
 *
 */
static PyObject *Z80_load_memory(PyZ80 *self, PyObject *args, PyObject *kwargs) {
    int address;
    PyObject *data;
    Py_ssize_t len;
    char *bytes;

    static char *kwlist[] = {"data", "address", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Si", kwlist, &data, &address))
        return NULL;

    // TODO: check address range and data length
    address &= MEMORY_MASK;
    len = PyString_Size(data);
    bytes = PyString_AsString(data);
    TRACE("load_memory at %04x, %i bytes\n", address, len);
    for (int i = 0; (i < len) && (address < MEMORY_SIZE); i++, address++) {
        self->context.memory[address] = bytes[i];
    }

    Py_RETURN_NONE;
}

/*
 * Write memory address.
 *
 *   poke(address, value)
 *
 * address: integer (0-65536)
 * value: integer (0-255)
 *
 */
static PyObject *Z80_poke(PyZ80 *self, PyObject *args, PyObject *kwargs) {
    int address;
    int value;

    static char *kwlist[] = {"address", "value", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii", kwlist, &address, &value))
        return NULL;

    // TODO: check address and value range
    self->context.memory[address & 0xFFFF] = value & 0xFF;

    TRACE("poke(%04x, %02x)\n", address & 0xFFFF, value & 0xFF);
    Py_RETURN_NONE;
}

/*
 * Read memory address.
 *
 *   peek(address) -> value
 *
 * address: integer (0-65536)
 * value: integer (0-255)
 *
 */
static PyObject *Z80_peek(PyZ80 *self, PyObject *args, PyObject *kwargs) {
    static char *kwlist[] = {"address", NULL};
    int address;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &address))
        return NULL;

    // TODO: check address range
    TRACE("peek(%04x) => %02x\n", address & 0xFFFF,
          self->context.memory[address & 0xFFFF]);
    return PyInt_FromLong(self->context.memory[address & 0xFFFF]);
}

/*
 * Load register.
 *
 *   load_register(name, value)
 *
 * name: string, uppercase
 * value: integer (0-255 o 0-65535)
 *
 */
static PyObject *Z80_load_register(PyZ80 *self, PyObject *args, PyObject *kwargs) {
    int value;
    PyObject *name;
    const TRegisterMapping *p;

    static char *kwlist[] = {"register", "value", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Si", kwlist, &name, &value))
        return NULL;

    p = lookup_register(PyString_AsString(name));
    if (p == NULL) {
        // TODO: set exception
        TRACE("register not found: %s\n", PyString_AsString(name));
        return NULL;
    }

    // TODO: check value range
    switch (p->dw) {
    case 0:
        self->state.registers.byte[p->index] = value & 0xFF;
        TRACE("load_register: %s = %02x\n", p->name, value & 0xFF);
        break;

    case 1:
        self->state.registers.word[p->index] = value & 0xFFFF;
        TRACE("load_register: %s = %04x\n", p->name, value & 0xFFFF);
        break;

    case 2:
        self->state.pc = value &0xFFFF;
        break;
    }

    Py_RETURN_NONE;
}


static PyGetSetDef Z80_getseters[] = {
    {"memory",  (getter)Z80_getmemory,  NULL, "memory buffer", NULL},
    {"is_done", (getter)Z80_get_isdone, NULL, "done flag",     NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef Z80_members[] = {
    /* {"first", T_OBJECT_EX, offsetof(Noddy, first), 0, */
    /*  "first name"}, */
    /* {"last", T_OBJECT_EX, offsetof(Noddy, last), 0, */
    /*  "last name"}, */
    /* {"number", T_INT, offsetof(Noddy, number), 0, */
    /*  "noddy number"}, */
    {NULL}  /* Sentinel */
};

static PyMethodDef Z80_methods[] = {
    {"reset", (PyCFunction)Z80_reset, METH_NOARGS,
     "Resets the Z80"},
    {"int", (PyCFunction)Z80_interrupt, METH_KEYWORDS,
     "Trigger an interruption"},
    {"nmi", (PyCFunction)Z80_non_maskable_interrupt, METH_NOARGS,
     "Trigger a non maskable interruption"},
    {"emulate", (PyCFunction)Z80_emulate, METH_KEYWORDS,
     "Run the emulation for a given numver of cycles"},
    {"load_memory", (PyCFunction)Z80_load_memory, METH_KEYWORDS,
     "Load the memory from a string"},
    {"poke", (PyCFunction)Z80_poke, METH_KEYWORDS,
     "Write a memory location"},
    {"peek", (PyCFunction)Z80_peek, METH_KEYWORDS,
     "Read a memory location"},
    {"load_register", (PyCFunction)Z80_load_register, METH_KEYWORDS,
     "Load a register"},
    {NULL}  /* Sentinel */
};

static PyTypeObject Z80_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyz80.Z80",               /* tp_name */
    sizeof(PyZ80),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)Z80_dealloc,   /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                               /* tp_flags */
    "Z80 objects",             /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Z80_methods,               /* tp_methods */
    Z80_members,               /* tp_members */
    Z80_getseters,             /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Z80_init,        /* tp_init */
    0,                         /* tp_alloc */
    Z80_new,                   /* tp_new */
};

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initpyz80(void) {
    PyObject* m;

    if (PyType_Ready(&Z80_type) < 0)
        return;

    m = Py_InitModule3("pyz80", module_methods, "Z80 extension type.");

    Py_INCREF(&Z80_type);
    PyModule_AddObject(m, "Z80", (PyObject *)&Z80_type);
}
