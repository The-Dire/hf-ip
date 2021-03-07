CPPFLAGS = -I ../include -Wall -Werror

all: libhfip

debug: CFLAGS+= -DDEBUG_API
debug: libhfip

libhfip: libhfip.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -shared -o libhfip.so libhfip.c -ldl

.PHONY:
clean:
	rm libhfip.so*
