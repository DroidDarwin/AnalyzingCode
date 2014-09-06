# -*- makefile -*-
# Makefile for mkmf.
# Makefile for Unix and GNU/Linux with gcc/g++ compiler
# Edit here:

CC=gcc
CPP=g++

# Hints:
# if you use egcs-2.90.* version of GCC please add option -fno-exceptions 
# to reduce code size and increase performance
# remove -DSLIST for really old systems without SGI's STL implementations
###############################################################################

# Debug version
#CFLAGS = -c -Wall -O0 -g  -DDUMP_EDGES -DDEBUG -DDUMP_MONITOR
#CFLAGS = -c -Wall -O0 -g  -DDUMP_EDGES -DDUMP_BYTE_CODES -DPRINT_PC -DDUMP_STACK -DDUMP_MONITOR -DDEBUG
# DSLIST DHASH_TABLE removed because it wouldn't compile otherwise on gcc 3.x


# Optimized version

CFLAGS = -c -Wall -O2 -g
# -DSLIST removed because it wouldn't compile under gcc 3.x
# add -DHASH_TABLE for extra speed (may sometimes produce inconsistent results)

# Optimized version with switched off asserts
#CFLAGS = -c -Wall -O2 -g -DSLIST -DNDEBUG

# link zlib the compression/decompression library. used for decompressing jar files
LFLAGS=-g -lz

# Directory to place executables
INSTALL_DIR=/usr/local/bin

# Version number
# VERSION=`grep VERSION jlint.hh | sed 's/.*N //'`
VERSION=3.1.2

# Files that go into distro
DISTFILES=`ls jlint-$(VERSION)/{antic.c,BUGS,Makefile,*.msg,*.hh,*.cc,*.d,README,TODO,CHANGELOG,COPYING,manual.texi,manual.html,manual.pdf,jlint.sh,mkmf.pl}`

TESTDISTFILES=`ls jlint-$(VERSION)/{antic.c,BUGS,Makefile,*.msg,*.hh,*.cc,*.d,README,TODO,CHANGELOG,COPYING,manual.texi,manual.html,manual.pdf,jlint_3.0.tex,jlint.sh,mkmf.pl,runtest.sh,showdiff.sh,showerror.sh,testall.sh,README.tests,tests.tar.bz2}`



# Makefile rules

all: antic jlint

antic.o: antic.c
	$(CC) $(CFLAGS) antic.c

antic: antic.o
	$(CC) $(LFLAGS) -o antic antic.o

clean: 
	rm -f  *.o *.exe core *~ *.his *.class jlint antic manual.{html,pdf,aux,cp,fn,ky,log,pg,toc,tp,vr} jlint_3.0.{aux,dvi,log,toc}
	if [ -d ./tests/log ]; then rm -f ./tests/log/*~ ./tests/log/*.{err,errfiles,log}; rm -f ./tests/log/test.{diff,err}; fi
	if [ -d ./tests ]; then rm -f ./tests/*~ tests.tar.bz2; fi

doc:  manual.texi
	texi2html -monolithic manual.texi; texi2pdf manual.texi

dist:  	doc tarbz targz zip
	chmod 644 ../jlint-*.{tar.bz2,tar.gz,zip}

tarbz:
	if [ $(VERSION) != `grep VERSION jlint.hh | sed 's/.*N //'` ]; then echo "Check version numbers!"; exit 1; fi
	if [ ! -e ../jlint-$(VERSION) ]; then ln -s $(PWD) ../jlint-$(VERSION); fi
	cd ..; tar -chv --bzip2 -f jlint-$(VERSION).tar.bz2 $(DISTFILES)

targz:
	if [ $(VERSION) != `grep VERSION jlint.hh | sed 's/.*N //'` ]; then echo "Check version numbers!"; exit 1; fi
	if [ ! -e ../jlint-$(VERSION) ]; then ln -s $(PWD) ../jlint-$(VERSION); fi
	cd ..; tar -chvzf jlint-$(VERSION).tar.gz $(DISTFILES)

zip:
	if [ $(VERSION) != `grep VERSION jlint.hh | sed 's/.*N //'` ]; then echo "Check version numbers!"; exit 1; fi	
	if [ ! -e ../jlint-$(VERSION) ]; then ln -s $(PWD) ../jlint-$(VERSION); fi
	cd ..; rm -f jlint-$(VERSION).zip; zip -v jlint-$(VERSION).zip $(DISTFILES) `find jlint-$(VERSION)/jlintwin32 | grep -v CVS`

test-dist:	doc test-files-tar test-tarbz test-targz zip
	chmod 644 ../jlint-*.{tar.bz2,tar.gz,zip}

test-files-tar:
	tar -chv --bzip2 -f tests.tar.bz2 tests/

test-tarbz:
	if [ $(VERSION) != `grep VERSION jlint.hh | sed 's/.*N //'` ]; then echo "Check version numbers!"; exit 1; fi
	if [ ! -e ../jlint-$(VERSION) ]; then ln -s $(PWD) ../jlint-$(VERSION); fi
	cd ..; tar -chv --bzip2 -f jlint-$(VERSION).tar.bz2 $(TESTDISTFILES)

test-targz:
	if [ $(VERSION) != `grep VERSION jlint.hh | sed 's/.*N //'` ]; then echo "Check version numbers!"; exit 1; fi
	if [ ! -e ../jlint-$(VERSION) ]; then ln -s $(PWD) ../jlint-$(VERSION); fi
	cd ..; tar -chvzf jlint-$(VERSION).tar.gz $(TESTDISTFILES)

test-zip:
	if [ $(VERSION) != `grep VERSION jlint.hh | sed 's/.*N //'` ]; then echo "Check version numbers!"; exit 1; fi
	if [ ! -e ../jlint-$(VERSION) ]; then ln -s $(PWD) ../jlint-$(VERSION); fi
	cd ..; rm -f jlint-$(VERSION).zip; zip -v jlint-$(VERSION).zip $(TESTDISTFILES) `find jlint-$(VERSION)/jlintwin32 | grep -v CVS`


install:
	cp jlint antic jlint.sh $(INSTALL_DIR)
	chmod 755 $(INSTALL_DIR)/antic
	chmod 755 $(INSTALL_DIR)/jlint
	chmod 755 $(INSTALL_DIR)/jlint.sh


# --> automatically generated dependencies follow; do not remove this line.
jlint: \
	access_desc.o \
	callee_desc.o \
	class_desc.o \
	graph.o \
	jlint.o \
	local_context.o \
	locks.o \
	message_node.o \
	method_desc.o
	$(CPP) -o jlint access_desc.o callee_desc.o class_desc.o graph.o jlint.o local_context.o locks.o message_node.o method_desc.o $(LFLAGS)

access_desc.o: access_desc.cc \
	access_desc.hh \
	class_desc.hh \
	functions.hh \
	types.hh \
	locks.hh \
	utf_string.hh \
	field_desc.hh \
	graph.hh \
	method_desc.hh \
	overridden_method.hh \
	jlint.d \
	jlint.msg \
	message_node.hh \
	component_desc.hh \
	inlines.hh \
	var_desc.hh \
	constant.hh \
	callee_desc.hh \
	local_context.hh \
	string_pool.hh
	$(CPP) $(CFLAGS) access_desc.cc

callee_desc.o: callee_desc.cc \
	callee_desc.hh \
	class_desc.hh \
	functions.hh \
	types.hh \
	locks.hh \
	utf_string.hh \
	field_desc.hh \
	graph.hh \
	method_desc.hh \
	overridden_method.hh \
	jlint.d \
	jlint.msg \
	message_node.hh \
	component_desc.hh \
	inlines.hh \
	var_desc.hh \
	constant.hh \
	local_context.hh \
	access_desc.hh \
	string_pool.hh
	$(CPP) $(CFLAGS) callee_desc.cc

class_desc.o: class_desc.cc \
	class_desc.hh \
	types.hh \
	locks.hh \
	utf_string.hh \
	field_desc.hh \
	graph.hh \
	method_desc.hh \
	overridden_method.hh \
	jlint.d \
	jlint.msg \
	functions.hh \
	message_node.hh \
	component_desc.hh \
	inlines.hh \
	var_desc.hh \
	constant.hh \
	callee_desc.hh \
	local_context.hh \
	access_desc.hh \
	string_pool.hh
	$(CPP) $(CFLAGS) class_desc.cc

graph.o: graph.cc \
	graph.hh \
	types.hh \
	method_desc.hh \
	jlint.d \
	jlint.msg \
	inlines.hh \
	component_desc.hh \
	var_desc.hh \
	constant.hh \
	class_desc.hh \
	callee_desc.hh \
	field_desc.hh \
	local_context.hh \
	functions.hh \
	access_desc.hh \
	string_pool.hh \
	locks.hh \
	utf_string.hh \
	message_node.hh \
	overridden_method.hh
	$(CPP) $(CFLAGS) graph.cc

jlint.o: jlint.cc \
	jlint.hh \
	jlint.msg \
	types.hh \
	message_node.hh \
	utf_string.hh \
	method_desc.hh \
	field_desc.hh \
	class_desc.hh \
	constant.hh \
	callee_desc.hh \
	access_desc.hh \
	graph.hh \
	component_desc.hh \
	var_desc.hh \
	local_context.hh \
	overridden_method.hh \
	string_pool.hh \
	jlint.d \
	functions.hh \
	inlines.hh \
	locks.hh
	$(CPP) $(CFLAGS) jlint.cc

local_context.o: local_context.cc \
	local_context.hh \
	types.hh \
	var_desc.hh \
	method_desc.hh \
	jlint.d \
	jlint.msg \
	utf_string.hh \
	functions.hh \
	message_node.hh \
	inlines.hh \
	component_desc.hh \
	constant.hh \
	class_desc.hh \
	callee_desc.hh \
	field_desc.hh \
	access_desc.hh \
	string_pool.hh \
	locks.hh \
	graph.hh \
	overridden_method.hh
	$(CPP) $(CFLAGS) local_context.cc

locks.o: locks.cc \
	locks.hh \
	types.hh \
	field_desc.hh \
	jlint.d \
	jlint.msg \
	component_desc.hh \
	utf_string.hh \
	functions.hh \
	message_node.hh
	$(CPP) $(CFLAGS) locks.cc

message_node.o: message_node.cc \
	message_node.hh \
	functions.hh \
	types.hh \
	jlint.d \
	jlint.msg
	$(CPP) $(CFLAGS) message_node.cc

method_desc.o: method_desc.cc \
	method_desc.hh \
	types.hh \
	inlines.hh \
	component_desc.hh \
	var_desc.hh \
	constant.hh \
	class_desc.hh \
	callee_desc.hh \
	field_desc.hh \
	local_context.hh \
	functions.hh \
	access_desc.hh \
	string_pool.hh \
	locks.hh \
	jlint.d \
	jlint.msg \
	utf_string.hh \
	message_node.hh \
	graph.hh \
	overridden_method.hh
	$(CPP) $(CFLAGS) method_desc.cc

# --> end of automatically generated dependencies; do not remove this line.
