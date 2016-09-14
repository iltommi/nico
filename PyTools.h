
// -------------------
// Some Python helper functions
// -------------------

#ifndef PYHelper_H
#define PYHelper_H


#if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
//#warning check for fix here: https://bugs.python.org/issue17120
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#endif


#include <Python.h>
#include <vector>
#include <sstream>
#include <iostream>

#define ERROR(__txt) std::cout << "[ERROR] " << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ") " << __txt << std::endl;


//! tools to query python nemlist and get back C++ values and vectors
class PyTools {
private:
    //! convert Python object to bool
    static bool pyconvert(PyObject* py_val, bool &val) {
        int result;
        if (py_val) {
            result = PyObject_IsTrue(py_val);
            if( result == -1 ) return false;
            val = (bool) result;
            return true;
        }
        return false;
    }
    
    //! convert Python object to any integer type (short, uint, int, etc.)
    template<typename T>
    static bool pyconvert(PyObject* py_val, T &val) {
        if (py_val && PyNumber_Check(py_val)) {
            PyObject * v = PyNumber_Long(py_val);
            val=(T) PyLong_AsLong(v);
            Py_XDECREF(v);
            return true;
        }
        return false;
    }
    
    //! convert Python object to double
    static bool pyconvert(PyObject* py_val, double &val) {
        if (py_val && PyNumber_Check(py_val)) {
            PyObject * v = PyNumber_Float(py_val);
            val=(double) PyFloat_AsDouble(v);
            Py_XDECREF(v);
            return true;
        }
        return false;
    }
    
    //! convert Python object to string
    static bool pyconvert(PyObject* py_val, std::string &val) {
        if (py_val) {
            PyObject * s = NULL;
            if(    PyUnicode_Check(py_val)) s = PyUnicode_AsUTF8String(py_val); // python3
            else if( PyBytes_Check(py_val)) s = PyObject_Bytes(py_val); // python2
            if(s) {
                val=std::string(PyBytes_AsString(s));
                Py_XDECREF(s);
                return true;
            }
        }
        return false;
    }
    
    //! check error and display message
    static double get_py_result(PyObject* pyresult) {
        checkPyError();
        double cppresult=0;
        if (pyresult) {
            if(!convert(pyresult,cppresult)) {
                PyObject *ptype, *pvalue, *ptraceback;
                PyErr_Fetch(&ptype, &pvalue, &ptraceback);
                ERROR("function does not return float but " << pyresult->ob_type->tp_name);
            }
        } else {
            // we should never reach this point... something is weird
            ERROR("Function does not return a valid Python object");
        }
        return cppresult;
    }

public:

    static void openPython() {
        if (!Py_IsInitialized())
            Py_Initialize();
    }
    
    static void closePython() {
        if (Py_IsInitialized())
            Py_Finalize();
    }
    
    static std::string python_version()
    {
        std::string version;
        PyObject* platform = PyImport_ImportModule("platform");
        PyObject* pyversion = PyObject_CallMethod(platform, const_cast<char*>("python_version"), NULL);
        pyconvert(pyversion, version);
        Py_XDECREF(pyversion);
        Py_XDECREF(platform);
        PyErr_Print();
        return version;
    }
    
    //! convert Python object to C++ value
    template <typename T>
    static bool convert(PyObject* py_val, T &val) {
        bool retval=pyconvert(py_val, val);
        return retval;
    }
    
    //! convert vector of Python objects to vector of C++ values
    template <typename T>
    static bool convert(std::vector<PyObject*> py_vec, std::vector<T> &val) {
        bool retval=true;
        val.resize(py_vec.size());
        for (unsigned int i=0;i<py_vec.size();i++) {
            bool thisval=convert(py_vec[i], val[i]);
            if (thisval==false) retval=false;
        }
        return retval;
    }
    
    //! convert python list to vector of python objects
    static bool convert(PyObject * py_list, std::vector<PyObject*> & py_vec) {
        if (py_list) {
            if (PyList_Check(py_list)) {
                PyObject* seq = PySequence_Fast(py_list, "expected a sequence");
                int len = PySequence_Size(seq);
                py_vec.resize(len);
                for (int i = 0; i < len; i++) {
                    PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
                    py_vec[i]=item;
                }
                Py_DECREF(seq);
                return true; // success
            }
        }
        PyTools::checkPyError();
        return false; // error
    }
    
    //! convert python list to vector of c++ values
    template <typename T>
    static bool convert(PyObject * py_list, std::vector<T> & vec) {
        std::vector<PyObject*> py_vec;
        return convert(py_list, py_vec) && convert(py_vec, vec);
    }
    
    //! check if there has been a python error
    static void checkPyError(bool exitOnError=false) {
        if (PyErr_Occurred()) {
            PyObject *type, *value, *traceback;
            PyErr_Fetch(&type, &value, &traceback);
            PyErr_Clear();
            
            std::string message("");
            if (type) {
                std::string str("");
                PyObject * tn = PyObject_GetAttrString(type, "__name__");
                pyconvert(tn, str);
                Py_XDECREF(tn);
                message += str;
            }
            if (value) {
                message += ": ";
                message += PyTools::repr(value);
            }
            Py_XDECREF(type);
            Py_XDECREF(value);
            Py_XDECREF(traceback);
            if (exitOnError) {
                ERROR(message);
            }
        }
    }
    
    //! run void python function
    static void runPyFunction(std::string name) {
        PyObject* myFunction = PyObject_GetAttrString(PyImport_AddModule("__main__"),name.c_str());
        if (myFunction) {
            PyObject_CallFunction(myFunction,const_cast<char *>(""));
            checkPyError(true);
            Py_DECREF(myFunction);
        } else {
            ERROR("python " << name << " function does not exists");
        }
    }
    
    //! run typed python function without arguments
    template <typename T=double>
    static T runPyFunction(std::string name, std::string component=std::string("")) {
        T retval(0);
        PyObject *py_obj=PyImport_AddModule("__main__");
        if (!component.empty()) {
            py_obj = PyObject_GetAttrString(py_obj,component.c_str());
            PyTools::checkPyError();
        }
        PyObject* myFunction = PyObject_GetAttrString(py_obj,name.c_str());
        if (myFunction) {
            PyObject *pyresult = PyObject_CallFunction(myFunction, const_cast<char *>(""));
            retval = (T) get_py_result(pyresult);
            Py_DECREF(myFunction);
            Py_XDECREF(pyresult);
        }
        return retval;
    }
    
    //! run typed python function with one argument
    template <typename T=double>
    static T runPyFunction(PyObject *pyFunction, double x1) {
        PyObject *pyresult = PyObject_CallFunction(pyFunction, const_cast<char *>("d"), x1);
        T retval = (T) get_py_result(pyresult);
        Py_XDECREF(pyresult);
        return retval;
    }
    
    //! run typed python function with two arguments
    template <typename T=double>
    static T runPyFunction(PyObject *pyFunction, double x1, double x2) {
        PyObject *pyresult = PyObject_CallFunction(pyFunction, const_cast<char *>("dd"), x1, x2);
        T retval = (T) get_py_result(pyresult);
        Py_XDECREF(pyresult);
        return retval;
    }
    
    //! run typed python function with three arguments
    template <typename T=double>
    static T runPyFunction(PyObject *pyFunction, double x1, double x2, double x3) {
        PyObject *pyresult = PyObject_CallFunction(pyFunction, const_cast<char *>("ddd"), x1, x2, x3);
        T retval = (T) get_py_result(pyresult);
        Py_XDECREF(pyresult);
        return retval;
    }
    
    //! run typed python function with four arguments
    template <typename T=double>
    static T runPyFunction(PyObject *pyFunction, double x1, double x2, double x3, double x4) {
        PyObject *pyresult = PyObject_CallFunction(pyFunction, const_cast<char *>("dddd"), x1, x2, x3, x4);
        T retval = (T) get_py_result(pyresult);
        Py_XDECREF(pyresult);
        return retval;
    }
    
    //! get T from python
    template< typename T>
    static bool extract(std::string name, T &val, std::string component=std::string(""), int nComponent=0) {
        PyObject* py_val = extract_py(name,component,nComponent);
        PyTools::checkPyError();
        if (PyList_Check(py_val)) {
            ERROR("Looking for single value \"" << name << "\" in " << component << " #" << nComponent << " but got a list.");
        }
        return PyTools::convert(py_val,val);
    }
    
    //! extract vectors
    template< typename T>
    static bool extract(std::string name, std::vector<T> &val, std::string component=std::string(""), int nComponent=0) {
        std::vector<PyObject*> py_val = extract_pyVec(name,component,nComponent);
        if (py_val.size())
            return PyTools::convert(py_val,val);
        return false;
    }
    
    //! retrieve python object
    static PyObject* extract_py(std::string name, std::string component=std::string(""), int nComponent=0) {
        if (name.find(" ")!= std::string::npos || component.find(" ")!= std::string::npos) {
            ERROR("asking for [" << name << "] [" << component << "] : it has whitespace inside: please fix the code");
        }
        if (!Py_IsInitialized()) ERROR("Python not initialized: this should not happen");
        PyObject *py_obj=PyImport_AddModule("__main__");
        // If component requested
        if (!component.empty()) {
            // Get the selected component (e.g. "Species" or "Laser")
            py_obj = PyObject_GetAttrString(py_obj,component.c_str());
            PyTools::checkPyError();
            // Error if not found
            if (!py_obj) ERROR("Component "<<component<<" not found in namelist");
            // If successfully found
            int len = PyObject_Length(py_obj);
            if (len > nComponent) {
                py_obj = PySequence_GetItem(py_obj, nComponent);
            } else {
                ERROR("Requested " << component << " #" <<nComponent<< ", but only "<<len<<" available");
            }
        }
        PyObject *py_return=PyObject_GetAttrString(py_obj,name.c_str());
        PyTools::checkPyError();
        return py_return;
    }
    
    //! retrieve a vector of python objects
    static std::vector<PyObject*> extract_pyVec(std::string name, std::string component=std::string(""), int nComponent=0) {
        std::vector<PyObject*> retvec;
        PyObject* py_obj = extract_py(name,component,nComponent);
        if( ! convert(py_obj, retvec) ) {
            std::ostringstream ss("");
            if (component!="") {
                ss << "In " << component << " #" << nComponent << " ";
            }
            ERROR( ss.str() << name << " should be a list not a scalar: use [...]");
        }
        return retvec;
    }
    
    static bool extract_pyProfile(std::string name, PyObject*& myPy, std::string component=std::string(""), int nComponent=0) {
        PyObject* myPytmp=extract_py(name,component,nComponent);
        if (PyCallable_Check(myPytmp)) {
            myPy=myPytmp;
            return true;
        }
        return false;
    }
    
    // extract 3 profiles from namelist (used for part mean velocity and temperature)
    static void extract3Profiles(std::string varname, int ispec, PyObject*& profx, PyObject*& profy, PyObject*& profz )
    {
        std::vector<PyObject*> pvec = PyTools::extract_pyVec(varname,"Species",ispec);
        if ( pvec.size()==1 ) {
            if( !PyCallable_Check(pvec[0]) )
                ERROR("For species #" << ispec << ", "<<varname<<" not understood");
            profx =  profy =  profz = pvec[0];
        } else if (pvec.size()==3) {
            if( !PyCallable_Check(pvec[0]) || !PyCallable_Check(pvec[1]) || !PyCallable_Check(pvec[2]) )
                ERROR("For species #" << ispec << ", "<<varname<<" not understood");
            profx = pvec[0];
            profy = pvec[1];
            profz = pvec[2];
        } else {
            ERROR("For species #" << ispec << ", "<<varname<<" needs 1 or 3 components.");
        }
    }
    
    // extract 2 profiles from namelist (used for laser profile)
    static bool extract2Profiles(std::string varname, int ilaser, std::vector<PyObject*> &profiles )
    {
        PyObject* py_obj = extract_py(varname,"Laser",ilaser);
        // Return false if None
        if( py_obj==Py_None ) return false;
        
        // Error if not list
        if( ! convert(py_obj, profiles) )
            ERROR("For laser #" << ilaser << ": " << varname << " must be a list of 2 profiles");
        
        // Error if wrong size
        if( profiles.size()!=2 )
            ERROR("For Laser #" << ilaser << ": "<<varname<<" needs 2 profiles.");
        
        // Error if not callable
        for( int i=0; i<2; i++) {
            if ( !PyCallable_Check(profiles[i]) )
                ERROR("For Laser #" << ilaser << ": "<<varname<<"["<<i<<"] not understood");
        }
        
        return true;
    }
    
    //! return the number of components (see pyinit.py)
    static unsigned int nComponents(std::string componentName) {
        // Get the selected component (e.g. "Species" or "Laser")
        if (!Py_IsInitialized()) ERROR("Python not initialized: this should not happen");
        PyObject *py_obj = PyObject_GetAttrString(PyImport_AddModule("__main__"),componentName.c_str());
        PyTools::checkPyError();
        Py_ssize_t retval = PyObject_Length(py_obj);
        if (retval < 0) {
            ERROR("Problem searching for component " << componentName);
        }
        return retval;
    }
    
    //! Get an object's attribute ( int, double, etc.)
    template <typename T>
    static bool getAttr(PyObject* object, std::string attr_name, T & value) {
        bool success = false;
        if( PyObject_HasAttrString(object, attr_name.c_str()) ) {
            PyObject* py_value = PyObject_GetAttrString(object, attr_name.c_str());
            success = convert(py_value, value);
            Py_XDECREF(py_value);
        }
        return success;
    }
    
    //! Get an object's attribute for lists
    template <typename T>
    static bool getAttr(PyObject* object, std::string attr_name, std::vector<T> & vec) {
        bool success = false;
        if( PyObject_HasAttrString(object, attr_name.c_str()) ) {
            PyObject* py_list = PyObject_GetAttrString(object, attr_name.c_str());
            success = convert(py_list, vec);
            Py_XDECREF(py_list);
        }
        return success;
    }
    
    //! Get an object's attribute for lists of list
    template <typename T>
    static bool getAttr(PyObject* object, std::string attr_name, std::vector<std::vector<T> > & vec) {
        if( PyObject_HasAttrString(object, attr_name.c_str()) ) {
            // Get the list of lists
            PyObject* py_list = PyObject_GetAttrString(object, attr_name.c_str());
            // Convert to vector of lists
            std::vector<PyObject*> py_vec;
            if( !convert(py_list, py_vec) ) return false;
            Py_XDECREF(py_list);
            // For each list, convert to vector
            vec.resize(0);
            std::vector<T> v;
            for( unsigned int i=0; i<py_vec.size(); i++ ) {
                if( !convert( py_vec[i], v ) ) return false;
                vec.push_back( v );
            }
        }
        //This should never happen
        return false;
    }
    
    //! Get an object's repr
    static std::string repr(PyObject* obj) {
        PyObject *s = PyObject_Str(obj);
        std::string str("");
        pyconvert(s,str);
        Py_XDECREF(s);
        return str;
    }
    
};

#endif
