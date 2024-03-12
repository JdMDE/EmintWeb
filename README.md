# EmintWeb
A framework for the development of embedded interactive web applications

This program is still in a preliminary version and has been tested only in Linux.

It depends on third-party libraries, concretely the gumbo HTML5 parser (package gumbo-parser-devel in Fedora).

EimitWeb is a framework that generates and compiles C++ code. The framework itself does not depend on anything else except gumbo but the generated code uses several of the Poco libraries (concretely: libPocoNet, libPocoUtil, libPocoFoundation and libPocoNetSSL) as long as libpthread so you will have to install them to compile and run the generated applications. 

To compile EmintWeb, first execute qmake-qt5 -o Makefile EmIntWeb.pro.
In Linux, it might be convenient to execute qmake-qt5 -spec /usr/lib64/qt5/mkspecs/linux-g++-64 -o Makefile EmIntWeb.pro depending on your distribution.

This will create file Makefile.

Run make, which in turn will create additional Makefiles (Makefile.eiwcli  Makefile.emintweb  Makefile.libht5cplus) for the three parts of this project (the command-line interface, the GUI and a needed library, libhtcplus) and will compile all of them.

Finally, and as root or a user with appropriate permissions, execute ./install.

The install macro just calls make install to put the library in /usr/local/lib and the executable programs (eiwcli and emintweb) in /usr/local/bin and copies some additional files (code skeletons, cursors and xsd files) in subdirectories of /usr/share/emintweb.
