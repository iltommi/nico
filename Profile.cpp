#include <cmath>

#include "Profile.h"
#include "PyTools.h"

using namespace std;



// Default constructor.
Profile::Profile(PyObject* py_profile) {
    
    if (!PyCallable_Check(py_profile)) {
        ERROR("Profile: not a function");
    }
    
    string message;
    
    // Verify that the profile has the right number of arguments
    PyObject* inspect=PyImport_ImportModule("inspect");
    PyTools::checkPyError();
    PyObject *tuple = PyObject_CallMethod(inspect,const_cast<char *>("getargspec"),const_cast<char *>("(O)"),py_profile);
    PyObject *arglist = PyTuple_GetItem(tuple,0);
    int size = PyObject_Size(arglist);
    
    Py_XDECREF(tuple);
    Py_XDECREF(inspect);
    
    // Assign the evaluating function, which depends on the number of arguments
    if      ( size == 1 ) function = new Function_Python1D(py_profile);
    else if ( size == 2 ) function = new Function_Python2D(py_profile);
    else if ( size == 3 ) function = new Function_Python3D(py_profile);
    else {
        ERROR("Profile: defined with unsupported number of variables");
    }
}


Profile::~Profile()
{
    delete function;
}


// Functions to evaluate a python function with various numbers of arguments
// 1D
double Function_Python1D::valueAt(vector<double> x_cell) {
    return PyTools::runPyFunction(py_profile, x_cell[0]);
}
double Function_Python2D::valueAt(vector<double> x_cell) {
    return PyTools::runPyFunction(py_profile, x_cell[0], x_cell[1]);
}
double Function_Python3D::valueAt(vector<double> x_cell) {
    return PyTools::runPyFunction(py_profile, x_cell[0], x_cell[1], x_cell[2]);
}
