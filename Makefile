EXEC=OGSSim
DOCDIR=docs
DOCFILE=$(DOCDIR)/Doxyfile
LIBDIR=lib/libzmq

.PHONY: all mrproper clean debug release

all: debug

debug:
	cd src; cmake ./ -DCMAKE_BUILD_TYPE=Debug; cd ..;
	$(MAKE) -C src
	cp src/${EXEC} ${EXEC}

release:
	cd src; cmake ./ -DCMAKE_BUILD_TYPE=Release; cd ..;
	$(MAKE) -C src
	cp src/${EXEC} ${EXEC}

clean:
	$(MAKE) -C src clean

mrproper:
	$(MAKE) -C src mrproper
	rm -f $(EXEC)
