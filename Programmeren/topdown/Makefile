PROGRAM = topdown.pl
CONFIG = topdown.xml
MAKEFILE = Makefile

INSTALL_PROGRAM = /usr/bin/topdown
INSTALL_CONFIG = /etc/$(CONFIG)

PERSONAL_CONFIG = ~/.$(CONFIG)

INTERPRETOR = perl
EXAMPLE_FILES = $(shell echo examples/*.cpp)
EXAMPLE_FILETYPE = pdf
EXAMPLE_OPTIONS = --remove --filetype $(EXAMPLE_FILETYPE)

PROGRAM_MODE = 0755
CONFIG_MODE = 0744

EXAMPLE_OUTPUT_FILES = $(addsuffix .$(EXAMPLE_FILETYPE), $(EXAMPLE_FILES))

# help - show useful targets
help:
	@echo Useful targets:
	@egrep "^# \w+ -" $(MAKEFILE) \
		| sed 's/^# \(\w\+\) - /    \1\t/'

# install - install the program
install: $(INSTALL_PROGRAM) $(INSTALL_CONFIG)

# config - create a personal config file
config: $(PERSONAL_CONFIG)

# examples - run the program on example files
examples: $(EXAMPLE_OUTPUT_FILES)

# uninstall - uninstall the program
uninstall:
	rm $(INSTALL_PROGRAM)
	rm $(INSTALL_CONFIG)

# unconfig - remove your personal config file
unconfig:
	rm ~/$(PERSONAL_CONFIG)

$(INSTALL_PROGRAM): $(PROGRAM) $(MAKEFILE)
	cp $< $@
	@chmod $(PROGRAM_MODE) $@

$(INSTALL_CONFIG): $(CONFIG) $(MAKEFILE)
	cp $< $@
	@chmod $(CONFIG_MODE) $@

$(PERSONAL_CONFIG): $(CONFIG) $(MAKEFILE)
	cp $< $@
	@chmod $(CONFIG_MODE) $@

define EXAMPLE_FILE
$1.$(EXAMPLE_FILETYPE): $(PROGRAM) $(MAKEFILE)
	$(INTERPRETOR) $(PROGRAM) $(EXAMPLE_OPTIONS) $1
endef

$(foreach example_file, $(EXAMPLE_FILES), \
	$(eval $(call EXAMPLE_FILE, $(example_file))))

.PHONY: help install config examples uninstall unconfig
