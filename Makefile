CC = gcc
# CXX variable is commonly used to define the C++ compiler
CXX = g++
ECHO = echo
RM = rm -f

CFLAGS = -Wall -Werror -ggdb
# We're using the same flags for both C and C++, but they could be different
CXXFLAGS = -Wall -Werror -ggdb
LDFLAGS = -lncurses

BIN =  rlg229
OBJS = rlg229.o dungeon.o move.o utils.o heap.o character.o pc.o npc.o io.o parser_objects.o

all: $(BIN) etags

$(BIN): $(OBJS)
	@$(ECHO) Linking $@
	@$(CXX) $^ -o $@ $(LDFLAGS)

-include $(OBJS:.o=.d)

%.o: %.c
	@$(ECHO) Compiling $<
	@$(CC) $(CFLAGS) -MMD -MF $*.d -c $<

# Copy the C rule, but change the extension and use CXX instead of CC.
# "Magic" going on here that I never really talked about in class:
# $< is the name of the input file.
# $* is the name of the input file with the extension stripped off.
# Starting commands with an '@' tells make not to echo the command (the
#   output of the command is still echoed.
# The '%' mean roughly the same as $*, but they're used in a different 
#   context.  Here it is used to describe how to make <base>.o from
#   <base>.cpp.
%.o: %.cpp
	@$(ECHO) Compiling $<
	@$(CXX) $(CXXFLAGS) -MMD -MF $*.d -c $<

.PHONY: clean clobber etags

clean:
	@$(ECHO) Removing all generated files
	@$(RM) *.o $(BIN) *.d TAGS core vgcore.*

clobber: clean
	@$(ECHO) Removing backup files
	@$(RM) *~ \#*

etags:
	@$(ECHO) Updating TAGS
	@etags *.[ch]
