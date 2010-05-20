PROGRAME_NAME = yu

CC := gcc
# gcc 编译默认优化级别会让很多变量不能用 gdb p 命令查看其值
# gdb 会出现 <value optimized out> 提示（自动优化了值存储方式）
# 用 -O0 可以便于调试
CFLAGS := -Wall -O0 -g

GTK_FLAGS = $(shell pkg-config gtk+-2.0 --cflags)
GTK_LIBS = $(shell pkg-config gtk+-2.0 --libs)

XML_FLAGS = $(shell pkg-config libxml-2.0 --cflags)
XML_LIBS = $(shell pkg-config libxml-2.0 --libs)

SSL_FLAGS = $(shell pkg-config libssl --cflags)
SSL_LIBS = $(shell pkg-config libssl --libs)

SQLITE3_FLAGS =
SQLITE3_LIBS = $(shell pkg-config sqlite3 --libs)

CURL_LIBS = -lcurl


all: yu

yu_object = yu.o opt.o usage.o dispatcher.o \
            load_config.o yu_error.o yu_debug.o dl.o \
            util.o checksum.o xml.o sql.o

checksum.o: checksum.h checksum.c
	$(CC) $(CFLAGS) $(SSL_FLAGS) -c checksum.c

xml.o: xml.h xml.c
	$(CC) $(CFLAGS) $(XML_FLAGS) -c xml.c

sql.o: sql.c sql.h
	$(CC) $(CFLAGS) $(SQLITE3_FLAGS) -c sql.c


commands_dir = commands
yu_commands = $(commands_dir)/download.o \
              $(commands_dir)/mirror.o


$(yu_object): yu.h opt.h usage.h dispatcher.h \
              load_config.h yu_error.h yu_debug.h dl.h \
              util.h

$(yu_commands): $(commands_dir)/common.h \
                $(commands_dir)/download.h \
                $(commands_dir)/mirror.h

# 注意，链接的时候不要重复写上 .o 文件
yu: $(yu_object) $(yu_commands) xml.h checksum.h
	$(CC) $(CFLAGS) -o $(PROGRAME_NAME) \
           $(yu_object) $(yu_commands) \
           $(CURL_LIBS) $(XML_LIBS)  $(SQLITE3_LIBS)



install:
	@echo "Have not a install feature"

.PHONY : clean

clean:
	-/bin/rm *.o \
           $(commands_dir)/*.o
	-/bin/rm $(PROGRAME_NAME)


