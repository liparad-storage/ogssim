README for OGSSim.

################################################################################
################################## -- INTRO -- #################################
################################################################################

OGSSim (Open and Generic storage System Simulation tool) is developped within
a collaboration project between the University of Versailles St Quentin and
the CEA.

OGSSim is developped by:
- S. Gougeaud (2014-)
- S. Zertal (2014-)

################################################################################
############################ -- ARCHIVE CONTENTS -- ############################
################################################################################

Directories:
- example: examples of system, traces and simulator configuration files
- include: OGSSim headers
- src: OGSSim sources

Files:
- Makefile
- OGSSim.1: manpage
- README.txt

################################################################################
################################## -- LIBS -- ##################################
################################################################################

Here is the list of needed libraries and their installation commands:

- cmake: build system
# apt-get install cmake

- doxygen: generate code documentation
# apt-get install doxygen

- libxerces-c: parse configuration files
# apt-get install libxerces-c-dev libxerces-c3.1

- libgoogle-glog: logging tool
# apt-get install libgoogle-glog-dev libgoogle-glog0

- 0MQ: allow communication between processes by using sockets to transmit
    information
# apt-get install libzmqpp3 libzmqpp-dev

- mathgl: chart creation
# apt-get install libmgl-dev

################################################################################
################################# -- HOW TO -- #################################
################################################################################

Compilation:
Two types of compilation are available: debug or release. The first one also
generates the log file. To launch the compilation:
$ make {debug|release}

Execution:
The execution command is:
$ ./OGSSim configurationFile

More information are available in the manpage OGSSim.1:
$ man ./OGSSim.1
