#include "Python.h"

#ifdef _MSC_VER
#include <Windows.h>
#include <stdio.h>
#include <DbgHelp.h>
/* Ignore unreferenced formal parameters. */
#pragma warning(disable: 4100)
/* Ignore "conditional expression is constant" in refcount macros. */
#pragma warning(disable: 4127)
#else
#error Sorry, Windows only.
#endif

HMODULE DbgHelp = NULL;
LPTOP_LEVEL_EXCEPTION_FILTER previous_filter = NULL;

PyObject *callback = NULL;

static struct {
    Py_UNICODE dir[MAX_PATH];
    Py_UNICODE app_name[MAX_PATH];
    MINIDUMP_TYPE dump_type;
} dump_details;

LONG WINAPI
exception_filter(EXCEPTION_POINTERS *exception)
{
    LONG rv = EXCEPTION_CONTINUE_SEARCH;
    HANDLE file;
    MINIDUMP_EXCEPTION_INFORMATION info;
    SYSTEMTIME st;
    Py_UNICODE name [MAX_PATH];

    GetSystemTime(&st);
    swprintf(name, MAX_PATH, L"%s\\%s_%04d%02d%02d-%02d%02d%02d.mdmp",
             dump_details.dir, dump_details.app_name,
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);
    file = CreateFileW(name, GENERIC_WRITE, /* not shared */0, NULL,
               CREATE_ALWAYS, /* Start fresh */
               FILE_FLAG_WRITE_THROUGH /* Write straigt to disk */,
               NULL);

    if (file == INVALID_HANDLE_VALUE)
        return rv;

    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = exception;
    info.ClientPointers = TRUE; /* Get the memory from this process. */

    rv = MiniDumpWriteDump(GetCurrentProcess(),
                           GetCurrentProcessId(),
                           file, dump_details.dump_type, &info,
                           0 /* User Stream */, 
                           0 /* Callback */);
    CloseHandle(file);

    if(callback)
        PyObject_CallObject(callback, Py_BuildValue("(u)", name));

    return EXCEPTION_EXECUTE_HANDLER;
}

PyDoc_STRVAR(enable_doc,
"enable([dir[, name[, type]]]) - Enable minidump creation on crashes.\n"
"\n"
"dir is the string name where the dump will be written.\n"
"The default directory is the current directory.\n"
"\n"
"name is the string name of your application, used as \n"
"the prefix of the dump name. The default is 'python'.\n"
"\n"
"type is an integer from the MINIDUMP_TYPE enum.\n"
"The default is MiniDumpNormal.\n"
"\n"
"callback is a function invoked after write coredump, dump file name as the only argument.");

static PyObject *
mdmp_enable(PyObject *self, PyObject *args, PyObject *kwargs)
{
    Py_UNICODE *pyname = L"python";
    Py_UNICODE *pydir = L"";
    int type = MiniDumpNormal;

    char *keywords[] = {"dir", "name", "type", "callback", NULL};

    if (DbgHelp == NULL) {
        if ((DbgHelp = LoadLibraryW(L"DbgHelp.dll")) == NULL) {
            PyErr_SetString(PyExc_OSError, "Unable to load DbgHelp");
            return NULL;
        }
    }

    if (!GetCurrentDirectoryW(/* buffer size*/ MAX_PATH, pydir)) {
        PyErr_SetString(PyExc_OSError, "Unable to get the current directory");
        return NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|uuiO:enable", keywords,
                                     &pydir, &pyname, &type, &callback)) {
        return NULL;
    }

    if (callback && !PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "callback must be callbable");
        return NULL;
    }

    wcsncpy(dump_details.app_name, pyname, wcslen(pyname));
    wcsncpy(dump_details.dir, pydir, wcslen(pydir));
    dump_details.dump_type = type;

    /* Store off the old filter so we can put it back later. */
    if (previous_filter != exception_filter)
        previous_filter = SetUnhandledExceptionFilter(exception_filter);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(disable_doc,
"disable() - Disable minidump creation on crashes.\n"
"\n"
"Reset the crash handler and unload DbgHelp.");

static PyObject *
mdmp_disable(PyObject *self)
{
    /* Be a good citizen and put the old handler back in. */
    if (previous_filter != NULL)
        SetUnhandledExceptionFilter(previous_filter);

    if (DbgHelp != NULL) {
        if (FreeLibrary(DbgHelp) == 0) {
            PyErr_SetString(PyExc_RuntimeError,
                            "Unable to unload DbgHelp.dll");
            return NULL;
        }
        DbgHelp = NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
mdmp_test(PyObject *mod)
{
    *(int*)0 = 0;

    Py_RETURN_NONE;
}

static PyMethodDef minidumper_functions[] = {
    {"enable", (PyCFunction)mdmp_enable,
               METH_VARARGS | METH_KEYWORDS, enable_doc},
    {"disable", (PyCFunction)mdmp_disable, METH_NOARGS, disable_doc},
    {"_test", (PyCFunction)mdmp_test, METH_NOARGS, "this is a test"},
    {NULL, NULL, 0, NULL}
};

//static struct PyModuleDef minidumper_module = {
//    PyModuleDef_HEAD_INIT,
//    "minidumper",
//    "Windows crash handler and minidump writer",
//    -1,
//    minidumper_functions,
//    NULL, NULL, NULL, NULL
//};

PyMODINIT_FUNC initminidumper(void)
{
    //PyObject *module = PyModule_Create(&minidumper_module);
    PyObject *module = Py_InitModule3("minidumper", minidumper_functions, "Windows crash handler and minidump writer");

    if (module == NULL)
        return NULL;

    /* MINIDUMP_TYPE values */
    PyModule_AddIntMacro(module, MiniDumpNormal);
    PyModule_AddIntMacro(module, MiniDumpWithDataSegs);
    PyModule_AddIntMacro(module, MiniDumpWithFullMemory);
    PyModule_AddIntMacro(module, MiniDumpWithHandleData);
    PyModule_AddIntMacro(module, MiniDumpFilterMemory);
    PyModule_AddIntMacro(module, MiniDumpScanMemory);
    PyModule_AddIntMacro(module, MiniDumpWithUnloadedModules);
    PyModule_AddIntMacro(module, MiniDumpWithIndirectlyReferencedMemory);
    PyModule_AddIntMacro(module, MiniDumpFilterModulePaths);
    PyModule_AddIntMacro(module, MiniDumpWithProcessThreadData);
    PyModule_AddIntMacro(module, MiniDumpWithPrivateReadWriteMemory);
    PyModule_AddIntMacro(module, MiniDumpWithoutOptionalData);
    PyModule_AddIntMacro(module, MiniDumpWithFullMemoryInfo);
    PyModule_AddIntMacro(module, MiniDumpWithThreadInfo);
    PyModule_AddIntMacro(module, MiniDumpWithCodeSegs);
    PyModule_AddIntMacro(module, MiniDumpWithoutAuxiliaryState);
    PyModule_AddIntMacro(module, MiniDumpWithFullAuxiliaryState);

    /* Perhaps all of these constants should follow the same idiom?
       The three below weren't found on one of my machines. */
#ifdef MiniDumpWithPrivateWriteCopyMemory
    PyModule_AddIntMacro(module, MiniDumpWithPrivateWriteCopyMemory);
#endif
#ifdef MiniDumpIgnoreInaccessibleMemory
    PyModule_AddIntMacro(module, MiniDumpIgnoreInaccessibleMemory);
#endif
#ifdef MiniDumpWithTokenInformation
    PyModule_AddIntMacro(module, MiniDumpWithTokenInformation);
#endif

    return module;
}


