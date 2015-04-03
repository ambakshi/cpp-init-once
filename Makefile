.PHONY: progs debug all run clean fmt install_hooks

UNAME_S:=$(shell uname -s |  cut -d'_' -f1)
ifeq ($(UNAME_S),CYGWIN)
sufexe=.exe
endif
CXX=clang
LD=clang
CXXFLAGS=-std=c++11
CPPFLAGS=-MD -MF $*.d
OPTFLAGS=-O3
LDFLAGS=-lstdc++

lastword = $(if $(firstword $1),$(word $(words $1),$1))
SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
PWD := $(shell pwd)

PROGS=atomic/atomic$(sufexe) init-once/init-oncei$(sufexe) win32/sfile$(sufexe)
SOURCES=atomic/atomic.cpp init-once/init-once.cpp win32/sfile.cpp
OBJS=$(patsubst %.cpp,%.o,$(SOURCES))
DEPS=$(patsubst %.cpp,%.d,$(SOURCES))

progs: $(PROGS)

debug:
	echo $(SELF_DIR) $(PWD)

run: $(PROGS)
	@for PROG in $^; do echo "######"; echo "Running $$PROG ..."; \
        time ./$$PROG ; \
    done

clean:
	rm -f $(PROGS) $(OBJS) $(DEPS)

fmt:
	 @SOURCES="$$(find . -name '*.[ch]pp')" && clang-format -i $$SOURCES

install_hooks:
	@ln -sfn ../../hooks/pre-commit-clang-format .git/hooks/pre-commit

-include $(DEPS)

atomic/atomic$(sufexe): atomic/atomic.o
	$(LD) $(LDFLAGS) $^ -o $@

init-once/init-once$(sufexe): init-once/init-once.o
	$(LD) $(LDFLAGS) $^ -o $@

win32/sfile$(sufexe): win32/sfile.o
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OPTFLAGS) -c $< -o $@

