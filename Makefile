################################################
# Author 	: Marco Caserta
# Date		: 02.01.08
################################################
BINDIR = bin
SRCDIR = src
# Question mark is used to read the name of the
# executable from shell (default name "dyn")
EXEC ?= dyn
# comment this line out to eliminate debug (gdb)
#DEBUG = -ggdb
CC        = g++
CCFLAGS   = -O3 -fomit-frame-pointer -pipe -Wreturn-type -Wcast-qual -Wpointer-arith -Wwrite-strings -DREPL

AUX_FILES = $(SRCDIR)/timer.cpp $(SRCDIR)/options.cpp $(SRCDIR)/heuristic.cpp
##############################################################
# this is used to compile the code for the cflp
default: $(SRCDIR)/containers.cpp
	@echo Creating $(BINDIR)/$(EXEC)
	$(CC) $(CCFLAGS) $(AUX_FILES) $(SRCDIR)/containers.cpp -o $(BINDIR)/$(EXEC)

##############################################################
# create doxygen documentation using "doxygen.conf" file
# the documentation is put into the directory Doc
doc: $(SRCDIR)/*.cpp $(AUX_FILES) doxygen/doxygen.conf
	doxygen doxygen/doxygen.conf
##############################################################

