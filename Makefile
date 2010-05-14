PROGRAME_NAME = yu
CC := gcc
CFLAGS := -Wall -O -g


all: yu


yu_object = yu.o opt.o usage.o dispatcher.o \
            load_config.o yu_error.o yu_debug.o

commands_dir = commands
yu_commands = $(commands_dir)/download.o


$(yu_object): yu.h opt.h usage.h dispatcher.h \
              load_config.h yu_error.h yu_debug.h

$(yu_commands): $(commands_dir)/common.h \
                $(commands_dir)/download.h


yu: $(yu_object) $(yu_commands)
	$(CC) $(CFLAGS) -o $(PROGRAME_NAME) \
           $(yu_object) $(yu_commands)


install:
	@echo "Have not a install feature"

.PHONY : clean

clean:
	-/bin/rm *.o \
           $(commands_dir)/*.o
	-/bin/rm $(PROGRAME_NAME)


