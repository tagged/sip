/*
 * The implementation of the different descriptors.
 *
 * Copyright (c) 2009 Riverbank Computing Limited <info@riverbankcomputing.com>
 * 
 * This file is part of SIP.
 * 
 * This copy of SIP is licensed for use under the terms of the SIP License
 * Agreement.  See the file LICENSE for more details.
 * 
 * SIP is supplied WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <Python.h>

#include "sip.h"
#include "sipint.h"


/*****************************************************************************
 * A method descriptor.  We don't use the similar Python descriptor because it
 * doesn't support a method having static and non-static overloads.
 *****************************************************************************/


/* Forward declarations of slots. */
static PyObject *sipMethodDescr_descr_get(PyObject *self, PyObject *obj,
        PyObject *type);


/*
 * The object data structure.
 */
typedef struct _sipMethodDescr {
    PyObject_HEAD

    /* The method definition. */
    PyMethodDef *pmd;
} sipMethodDescr;


/*
 * The type data structure.
 */
PyTypeObject sipMethodDescr_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sip.methoddescriptor", /* tp_name */
    sizeof (sipMethodDescr),    /* tp_basicsize */
    0,                      /* tp_itemsize */
    0,                      /* tp_dealloc */
    0,                      /* tp_print */
    0,                      /* tp_getattr */
    0,                      /* tp_setattr */
    0,                      /* tp_compare */
    0,                      /* tp_repr */
    0,                      /* tp_as_number */
    0,                      /* tp_as_sequence */
    0,                      /* tp_as_mapping */
    0,                      /* tp_hash */
    0,                      /* tp_call */
    0,                      /* tp_str */
    0,                      /* tp_getattro */
    0,                      /* tp_setattro */
    0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,     /* tp_flags */
    0,                      /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    0,                      /* tp_methods */
    0,                      /* tp_members */
    0,                      /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    sipMethodDescr_descr_get,   /* tp_descr_get */
};


/*
 * Return a new method descriptor for the given method.
 */
PyObject *sipMethodDescr_New(PyMethodDef *pmd)
{
    PyObject *descr = PyType_GenericAlloc(&sipMethodDescr_Type, 0);

    if (descr != NULL)
        ((sipMethodDescr *)descr)->pmd = pmd;

    return descr;
}


/*
 * The descriptor's descriptor get slot.
 */
static PyObject *sipMethodDescr_descr_get(PyObject *self, PyObject *obj,
        PyObject *type)
{
    sipMethodDescr *md = (sipMethodDescr *)self;

    if (obj == Py_None)
        obj = NULL;

    return PyCFunction_New(md->pmd, obj);
}


/*****************************************************************************
 * A variable descriptor.  We don't use the similar Python descriptor because
 * it doesn't support static variables.
 *****************************************************************************/


/* Forward declarations of slots. */
static PyObject *sipVariableDescr_descr_get(PyObject *self, PyObject *obj,
        PyObject *type);
static int sipVariableDescr_descr_set(PyObject *self, PyObject *obj,
        PyObject *value);


/*
 * The object data structure.
 */
typedef struct _sipVariableDescr {
    PyObject_HEAD

    /* The getter/setter definition. */
    sipVariableDef *vd;

    /* The generated type definition. */
    const sipClassTypeDef *ctd;
} sipVariableDescr;


/*
 * The type data structure.
 */
PyTypeObject sipVariableDescr_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "sip.variabledescriptor",   /* tp_name */
    sizeof (sipVariableDescr),  /* tp_basicsize */
    0,                      /* tp_itemsize */
    0,                      /* tp_dealloc */
    0,                      /* tp_print */
    0,                      /* tp_getattr */
    0,                      /* tp_setattr */
    0,                      /* tp_compare */
    0,                      /* tp_repr */
    0,                      /* tp_as_number */
    0,                      /* tp_as_sequence */
    0,                      /* tp_as_mapping */
    0,                      /* tp_hash */
    0,                      /* tp_call */
    0,                      /* tp_str */
    0,                      /* tp_getattro */
    0,                      /* tp_setattro */
    0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,     /* tp_flags */
    0,                      /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    0,                      /* tp_methods */
    0,                      /* tp_members */
    0,                      /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    sipVariableDescr_descr_get, /* tp_descr_get */
    sipVariableDescr_descr_set, /* tp_descr_set */
};


/* Forward declarations. */
static int get_instance_address(sipVariableDescr *vd, PyObject *obj,
        void **addrp);


/*
 * Return a new method descriptor for the given getter/setter.
 */
PyObject *sipVariableDescr_New(sipVariableDef *vd, sipClassTypeDef *ctd)
{
    PyObject *descr = PyType_GenericAlloc(&sipVariableDescr_Type, 0);

    if (descr != NULL)
    {
        ((sipVariableDescr *)descr)->vd = vd;
        ((sipVariableDescr *)descr)->ctd = ctd;
    }

    return descr;
}


/*
 * The descriptor's descriptor get slot.
 */
static PyObject *sipVariableDescr_descr_get(PyObject *self, PyObject *obj,
        PyObject *type)
{
    sipVariableDescr *vd = (sipVariableDescr *)self;
    void *addr;

    if (get_instance_address(vd, obj, &addr) < 0)
        return NULL;

    return vd->vd->vd_getter(addr, obj, type);
}


/*
 * The descriptor's descriptor set slot.
 */
static int sipVariableDescr_descr_set(PyObject *self, PyObject *obj,
        PyObject *value)
{
    sipVariableDescr *vd = (sipVariableDescr *)self;
    void *addr;

    /* Check that the value isn't const. */
    if (vd->vd->vd_setter == NULL)
    {
        PyErr_Format(PyExc_AttributeError,
                "'%s' object attribute '%s' is read-only",
                sipPyNameOfClass(vd->ctd), vd->vd->vd_name);

        return -1;
    }

    if (get_instance_address(vd, obj, &addr) < 0)
        return -1;

    return vd->vd->vd_setter(addr, obj, value);
}


/*
 * Return the C/C++ address of any instance.
 */
static int get_instance_address(sipVariableDescr *vd, PyObject *obj,
        void **addrp)
{
    void *addr;

    if (vd->vd->vd_is_static)
    {
        addr = NULL;
    }
    else
    {
        /* Check that access was via an instance. */
        if (obj == NULL || obj == Py_None)
        {
            PyErr_Format(PyExc_AttributeError,
                    "'%s' object attribute '%s' is an instance attribute",
                    sipPyNameOfClass(vd->ctd), vd->vd->vd_name);

            return -1;
        }

        /* Get the C++ instance. */
        if ((addr = sip_api_get_cpp_ptr((sipSimpleWrapper *)obj, vd->ctd)) == NULL)
            return -1;
    }

    *addrp = addr;

    return 0;
}
