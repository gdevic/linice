What is this?

This is the root project directory for the Linice Sim. If you have an MS Windows machine with MS Visual Studio available, you can run this simulation box and it will compile Linice within a simulated environment which will allow you to debug the debugger :)

Of course, it falls short of any kind of elaborate PC/CPU emulation, but for what is worth, it does an excellent job of helping to debug innards of Linice.

IMPORTANT! -
You have to set the environment variable LINICE_ROOT to the root directory where you mapped your Linice source installation. MSVC is using that environment variable to include various files.
Example: You may have Linice installed on your Linux machine and share the directory via Samba to your XP machine, mapped to drive L:, you would set LINICE_ROOT=L: in your system or user environment variables (System Properties tab), so when MSVC starts, the variable is already set.
Use MSVC 6.0 to open the workspace and compile.

There are two projects in the workspace:
* Linice - Debugger source files (kernel independent portion minus some Linux-specific files)
* Sim    - Simulator shell and GUI

Compile Linice to create a library; switch to Sim project and complete compilation. Run the Sim project. It will open a GUI with a simple simulated Linice debugging session.
- Select Simulation -> Install to "install" the debugger (initialize it)
- Select Simulation -> Load Symbol File if you want to debug Linice using an existing symbol file
- Select Events -> Int 1 to "break" into the simulated Linice
