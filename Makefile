#Program path configuration
OUTDIR	:= bin
SRCDIR	:= src
INCDIR	:= include
OBJDIR	:= obj
OUTBIN	:= $(OUTDIR)/server

#Compiler path configuration
CC		:= gcc

#Compiler flags
includes	= -I$(INCDIR) -I$(SRCDIR)
flags		= $(includes)
cflags		= $(flags) -O3
lflags		= $(flags)

#Files to compile from/to
c_files		= $(wildcard $(SRCDIR)/*.c)

#Target object files to link
o_files		= $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(notdir $(c_files))))

#Main target, the output executable
$(OUTBIN): $(o_files)
	$(CC) $(lflags) $^ -o $@

#Look into src folder for .c files to make corresponding .o files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(cflags) -c $< -o $@

all: $(OUTBIN)
default: all
clean:
	rm -f $(OUTBIN)
	rm -f $(OBJDIR)/*.o