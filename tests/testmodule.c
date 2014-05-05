#include "Python.h"

#include <Windows.h>

#pragma warning(disable: 4723)

void
divide_by_zero()
{
    int x = 1;
    int b = x / 0;
}

void
access_violation()
{
    *(int*)0 = 1;
}

static PyObject *
test_divide_by_zero(PyObject *self)
{
    divide_by_zero();
    Py_RETURN_NONE;
}

static PyObject *
test_access_violation(PyObject *self)
{
    access_violation();
    Py_RETURN_NONE;
}

static PyMethodDef tester_functions[] = {
    {"access_violation", (PyCFunction)test_access_violation,
                         METH_NOARGS, "this is a test"},
    {"divide_by_zero", (PyCFunction)test_divide_by_zero,
                         METH_NOARGS, "this is a test"},
    {NULL, NULL, 0, NULL}
};

//static struct PyModuleDef tester_module = {
//    PyModuleDef_HEAD_INIT,
//    "tester",
//    "Test module that crashes and does bad stuff",
//    -1,
//    tester_functions,
//    NULL, NULL, NULL, NULL
//};

PyMODINIT_FUNC inittester(void)
{
    //PyObject *module = PyModule_Create(&tester_module);
    PyObject *module = Py_InitModule3("tester", tester_functions, "Test module that crashes and does bad stuff");

    if (module == NULL)
        return;
}
