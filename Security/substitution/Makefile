MAIN_CLASS = org.hettoo.substitution.Main
SOURCE_DIR = source
BUILD_DIR = build
CLASS_DIR = $(BUILD_DIR)/classes

SOURCES = $(shell find $(SOURCE_DIR) -iname '*.java')
CLASSES = $(subst .java,.class,$(subst $(SOURCE_DIR),$(CLASS_DIR),$(SOURCES)))

COMMAND = java -cp $(CLASS_DIR) $(MAIN_CLASS)

all: classes

run: classes
	$(COMMAND)

classes: $(CLASSES)

$(CLASSES): $(CLASS_DIR) $(SOURCES)
	javac -Xlint:unchecked -d $(CLASS_DIR) $(SOURCES)

$(CLASS_DIR):
	mkdir -p $@

.PHONY: all run classes
