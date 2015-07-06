.PHONY: ../ruros.a
CXXFLAGS =	-O0 -ggdb3 -Wall -I ..

OBJS =		unix_io.o

all:	ruros-test.a

test_subdirs=$(wildcard test*)
test_subdirs_build=$(patsubst %,%.build,$(test_subdirs))
test_subdirs_run=$(patsubst %,%.run,$(test_subdirs))
test_subdirs_clean=$(patsubst %,%.clean,$(test_subdirs))

.PHONY: test%.build test%.run test%.clean

test%.build: ruros-test.a
	make -C $(patsubst %.build,%,$@)

test%.run: test%.build
	make -C $(patsubst %.run,%,$@) run

test%.clean:
	make -C $(patsubst %.clean,%,$@) clean

build: $(test_subdirs_build)

run: $(test_subdirs_run)
	@echo $(test_subdirs)
	@echo $(test_subdirs_x)

ruros-test.a: unix_io.o 
	ar cr $@ $^

../ruros.a:
	make -C ..

clean: $(test_subdirs_clean)
	rm -f $(OBJS) ruros-test.a
