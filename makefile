
CHUGINS=KnobGPIO

CHUGS=$(foreach CHUG,$(CHUGINS),$(CHUG)/$(CHUG).chug)
WEBCHUGS=$(foreach CHUG,$(CHUGINS),$(CHUG)/$(CHUG).chug.wasm)
CHUGS_WIN32=$(foreach CHUG,$(CHUGINS),$(CHUG)/$(CHUG).chug)
CHUGS_RELEASE=$(foreach CHUG,$(CHUGINS),$(CHUG)/Release/$(CHUG).chug)
CHUGS_CLEAN=$(addsuffix .clean,$(CHUGINS))

DESTDIR?=/usr/local
INSTALL_DIR=$(DESTDIR)/lib/chuck
INSTALL_DIR_WIN32="C:/Program Files/ChucK/chugins"

# default target: print usage message and quit
current: 
	@echo "[chugins build]: please use one of the following configurations:"
	@echo "   make linux, make mac, make web, or make win32"

ifneq ($(CK_TARGET),)
.DEFAULT_GOAL:=$(CK_TARGET)
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=$(.DEFAULT_GOAL)
endif
endif

CHUCK_STRICT=1

mac: $(CHUGS)
osx: $(CHUGS)
linux: $(CHUGS)
linux-alsa: $(CHUGS)
linux-jack: $(CHUGS)
web: $(WEBCHUGS)
win32: $(CHUGS_WIN32)

$(CHUGS):
	CHUCK_STRICT=1 make -C $(dir $@) $(MAKECMDGOALS)

$(WEBCHUGS):
	CHUCK_STRICT=1 make -C $(dir $@) $(MAKECMDGOALS)

clean: $(CHUGS_CLEAN)
.PHONY: $(CHUGS_CLEAN)
$(CHUGS_CLEAN):
	make -C $(basename $@) clean

install: $(CHUGS)
	mkdir -p $(INSTALL_DIR)
	cp -rf $(CHUGS) $(INSTALL_DIR)

install-win32: $(CHUGS_RELEASE)
	mkdir -p $(INSTALL_DIR_WIN32)
	cp -rf $(CHUGS_RELEASE) $(INSTALL_DIR_WIN32)

DATE=$(shell date +"%Y-%m-%d")

src-dist:
	mkdir -p chugins-src-$(DATE)/src/
	mkdir -p chugins-src-$(DATE)/examples/
	cp -rf $(EXAMPLES) chugins-src-$(DATE)/examples/
	git archive HEAD . | tar -x -C chugins-src-$(DATE)/src
	tar czf chugins-src-$(DATE).tgz chugins-src-$(DATE)


PPA_CHUG_VERSION?=1.3.5.0a
PPA_DEB_VERSION?=1.3.5.0a-ppa1

PPA_CHUG_DIR=chugins_$(PPA_CHUG_VERSION)
PPA_CHUG_TGZ=../chugins_$(PPA_CHUG_VERSION).orig.tar.gz

FORCE:

ppa-tgz: $(PPA_CHUG_TGZ)

ppa-source: $(PPA_CHUG_TGZ) ppa-clean
	debuild -S

$(PPA_CHUG_TGZ): FORCE
	rm -rf $(PPA_CHUG_DIR)
	mkdir -p $(PPA_CHUG_DIR)
	git archive HEAD . | tar -x -C $(PPA_CHUG_DIR)
	find $(PPA_CHUG_DIR)/ -type f -exec chmod a-x {} +
	tar czf $(PPA_CHUG_TGZ) $(PPA_CHUG_DIR)
	rm -rf $(PPA_CHUG_DIR)

ppa-binary: $(PPA_CHUG_TGZ) ppa-clean
	debuild -uc -us

ppa-upload:
	dput ppa:t-spencer/chuck ../chugins_$(PPA_DEB_VERSION)_source.changes

ppa-clean:
	debian/rules clean
	rm -rf debian/chuck

