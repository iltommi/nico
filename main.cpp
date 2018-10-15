#include "PyTools.h"
#include "Profile.h"
#include <iostream>
#include <list>
#include <string>

int main (int argc, char* argv[]) {
    
    PyTools::openPython();
    
    std::cout << "Python version: " << PyTools::python_version() << std::endl;

    PyTools::execFile(argv[1]);
    
    double my_pi=0;
    
    PyTools::extract("my_pi", my_pi);    
    std::cout<< "my_pi=" << my_pi << std::endl;

    Profile my_py_profile("my_func");
    for (int i=0; i<10; i++) {
        double retval=my_py_profile.valueAt(std::vector<double>{(double)i});
        std::cout<< "my_func("<<i<<")=" << retval << std::endl;
    }
    
    PyTools::closePython();
    return 0;
}
