# compiler options
CC = g++
DEBUG = -g
CFLAGS = -O2 -Wall -c $(DEBUG)
LFLAGS = -O2 -Wall $(DEBUG)

PROGRAM = charles_gtk

# normal objects
NORMAL_OBJS = globals world robot directed_point
# objects requiring gtk
GTK_OBJS = charles gui images

# per object extra local header dependencies
globals_depends = 
world_depends = robot
robot_depends = directed_point
directed_point_depends = globals
charles_depends = ../user globals world robot images gui
gui_depends = globals world robot images
images_depends = globals

# directories
SRC = src/
BUILD = build/

# configuration
GTK = $(shell pkg-config --cflags --libs gtk+-2.0)
OBJS = $(addprefix $(BUILD),$(addsuffix .o,user $(NORMAL_OBJS) $(GTK_OBJS)))
$(shell mkdir -p $(BUILD))

default: test

# target: help - show useful targets
help:
	@egrep "^# target:" [Mm]akefile | sed 's/^# target: //' | sed 's/^$$(PROGRAM)/$(PROGRAM)/'

# target: clean - clean up all eventually created files
clean:
	rm -f $(PROGRAM)
	rm -f $(BUILD)*.o
	rmdir $(BUILD)

# target: test - build the entire program and test it (default target)
test: $(PROGRAM)
	./$(PROGRAM)

# target: $(PROGRAM): build the entire program
$(PROGRAM): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(GTK) -o $@

$(BUILD)user.o: user.cpp user.hpp $(SRC)globals.hpp
	$(CC) $(CFLAGS) $< -o $@

define NORMAL_template
$(BUILD)$(1).o: $(SRC)$(1).cpp $(SRC)$(1).hpp $(addprefix $(SRC),$(addsuffix .hpp,$($(1)_depends)))
	$(CC) $(CFLAGS) $(SRC)$(1).cpp -o $(BUILD)$(1).o
endef

define GTK_template
$(BUILD)$(1).o: $(SRC)$(1).cpp $(SRC)$(1).hpp $(addprefix $(SRC),$(addsuffix .hpp,$($(1)_depends)))
	$(CC) $(CFLAGS) $(GTK) $(SRC)$(1).cpp -o $(BUILD)$(1).o
endef

$(foreach normal, $(NORMAL_OBJS), $(eval $(call NORMAL_template,$(normal))))
$(foreach gtk, $(GTK_OBJS), $(eval $(call GTK_template,$(gtk))))

.PHONY: default help clean test
