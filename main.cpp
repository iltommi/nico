#include "PyTools.h"
#include "Profile.h"
#include <iostream>
#include <list>
#include <string>
#include <fstream>

int main (int argc, char* argv[]) {
    if (argc!=2) return -1;
    
    PyTools::openPython();

    std::ifstream istr(argv[1]);
    if (!istr.is_open()) return -2;

    std::stringstream buffer;
    buffer << istr.rdbuf() << "\n";
        
    PyRun_SimpleString(buffer.str().c_str());
    
    double a=0;
    
    PyTools::extract("a", a);
    
    Profile my_py_profile("my_func");
    
    std::cout<< "a=" << a << std::endl;
    
    for (int i=0; i<10; i++) {
        double retval=my_py_profile.valueAt(std::vector<double>{(double)i});
        std::cout<< "my_func("<<i<<")=" << retval << std::endl;
    }
    
    PyTools::closePython();

    return 0;
}
