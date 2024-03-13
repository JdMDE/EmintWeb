# EmintWeb
A framework for the development of embedded interactive web applications

This program is still in a preliminary version and has been tested only in Linux (Fedora 38, Ubuntu and ArchLinux).

It depends on third-party libraries, concretely Qt5, the gumbo HTML5 parser (package gumbo-parser-devel in Fedora, libgumbo-dev in Ubuntu and gumbo-parser in ArchLinux).
Also, some Qt modules must be installed (look for the package names in your distribution attending to the compilation errors).

EimitWeb is a framework that generates and compiles C++ code. The framework itself does not depend on anything else except qt5 and gumbo but the generated code uses several of the Poco libraries (concretely: libPocoUtil, libPocoFoundation, libPocoNet and libPocoNetSSL) so you will have to install them to compile and run the generated applications. Also, the code will need libpthread and of course libc but these are present by default.

To compile EmintWeb, first execute qmake-qt5 -o Makefile EmIntWeb.pro.

This will create file Makefile.

Run make, which in turn will create additional Makefiles (Makefile.eiwcli  Makefile.emintweb  Makefile.libht5cplus) for the three parts of this project (the command-line interface, the GUI and a needed library, libhtcplus) and will compile all of them.

Finally, and as root or a user with appropriate permissions, execute ./install.

The install macro just calls make install to put the library in /usr/local/lib and the executable programs (eiwcli and emintweb) in /usr/local/bin and copies some additional files (code skeletons, cursors and xsd files) in subdirectories of /usr/share/emintweb.

The GUI can be called from the command line as emintweb.

The command line interface to emintweb is the program eiwcli. Run it without arguments to get help.
