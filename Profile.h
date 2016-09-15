#ifndef Profile_H
#define Profile_H

#include <vector>
#include <string>
#include "PyTools.h"

class Function
{
public:
    //! Default constructor
    Function(){};
    //! Default destructor
    virtual ~Function(){};
    // spatial
    virtual double valueAt(std::vector<double>) {
        return 0.;
    };
};


//  -------------------------------------------------------------------------------------------
//! Class Profile
//  -------------------------------------------------------------------------------------------
class Profile
{
public:
    //! Default constructor
    Profile(std::string name, std::string component=std::string(""), int nComponent=0);
    //! Default destructor
    ~Profile();
    
    //! Get the value of the profile at some location (spatial)
    inline double valueAt(std::vector<double> coordinates) {
        return function->valueAt(coordinates);
    };
    
private:
    //! Object that holds the information on the profile function
    Function * function;
    
};//END class Profile



// Children classes for python functions

class Function_Python1D : public Function
{
public:
    Function_Python1D(PyObject *pp) : py_profile(pp) {};
    double valueAt(std::vector<double>); // space
private:
    PyObject *py_profile;
};


class Function_Python2D : public Function
{
public:
    Function_Python2D(PyObject *pp) : py_profile(pp) {};
    double valueAt(std::vector<double>); // space
private:
    PyObject *py_profile;
};


class Function_Python3D : public Function
{
public:
    Function_Python3D(PyObject *pp) : py_profile(pp) {};
    double valueAt(std::vector<double>); // space
private:
    PyObject *py_profile;
};



#endif
