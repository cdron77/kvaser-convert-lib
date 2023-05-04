# Kvaser SDK conversion library

This project is based on **Kvaser CANLib SDK**. Original source is available at https://www.kvaser.com/download/.

The project goals are:

- Maintain a version which is buildable both on Linux (using *gcc* as compiler) and Windows (using *MinGW* as compiler).
- Build of *deb* packages for Debian-based distributions.
- Keep up to date with upstream code provided by Kvaser, but with git tracked source (source is currently only distributed as tarball).

The project contains:

- *linuxcan* folder with *Kvaser canlib* driver. It is not used directly by the project, but build as a dependency and used for header files.
- *kvlibsdk* The main support library from Kvaser. **NOTE:** Currently, only **kvlclib** and **kvadblib** libraries are built and maintained, mainly to support file conversion between CAN data acquisition files.
- *Makefile* Used to build Linux targets
- *Makefile.win32* Used to build Windows targets using MinGW.