# Linux/BSD/UNIX Makefile for Larn

MKDIR = mkdir -p
CP    = cp -r
MV    = mv
RM    = rm -f
RMDIR = rm -rf

# compiler & flags
CC      ?= cc
CFLAGS  += -Wall -Wextra -Werror -std=c17 -pedantic \
           -g -DNIX -DMULTIPLE_SCORE_ENTRY -DEXTRA

# platform libs
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -lncurses
else ifneq ($(UNAME_S),Windows)
    LDFLAGS += -lm -lncurses
endif

# directories
SRCDIR   := .
BUILDDIR := build
OBJDIR   := $(BUILDDIR)/obj
DEPDIR   := $(BUILDDIR)/dep
BINDIR   := bin
RUNTIME  := larnfiles/*

# sources, objects, deps
SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
DEP := $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$(OBJ))

TARGET := $(BINDIR)/larn

# build rules
all: $(TARGET)

$(TARGET): $(OBJ) | $(BINDIR)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

# create directories automatically
$(OBJDIR) $(DEPDIR) $(BINDIR):
	$(MKDIR) $(BUILDDIR)
	$(MKDIR) $(OBJDIR)
	$(MKDIR) $(DEPDIR)
	$(MKDIR) $(BINDIR)

# pattern rule for object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) $(DEPDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
	$(MV) $(OBJDIR)/$*.d $(DEPDIR)/$*.d

# include dependency files
-include $(DEP)

# copy runtime files into bin/
.PHONY: prep
prep: $(TARGET)
	$(MKDIR) $(BINDIR)
	$(CP) $(RUNTIME) $(BINDIR)

# cleaning
.PHONY: clean
clean:
	$(RMDIR) $(BUILDDIR)
	$(RM) $(TARGET)

.PHONY: distclean
distclean: clean
	$(RMDIR) $(BINDIR)
