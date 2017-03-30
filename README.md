README for OGSSim.

# Introduction

OGSSim (Open and Generic storage System Simulation tool) is developped within
a collaboration project between the University of Versailles St Quentin and
the CEA.

OGSSim is developped by:
- S. Gougeaud (2014-)
- S. Zertal (2014-)

# Repository contents

Directories:
- example: examples of system, traces and simulator configuration files
- include: OGSSim headers
- src: OGSSim sources

Files:
- Makefile
- OGSSim.1: manpage
- README.txt

# Library dependencies

Here is the list of needed libraries and their installation commands:

- cmake: build system
- libxerces-c: parse configuration files
- libgoogle-glog: logging tool
- 0MQ: allow communication between processes by using sockets to transmit
    information
- mathgl: chart creation

# How to

Compilation:
Two types of compilation are available: debug or release. The first one also
generates the log file. To launch the compilation:
$ make {debug|release}

Execution:
The execution command is:
$ ./OGSSim configurationFile

More information are available in the manpage OGSSim.1:
$ man ./OGSSim.1
