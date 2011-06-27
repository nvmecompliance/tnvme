SOURCES := tnvme.c
APP_NAME := tnvme

INCLUDE := $(PWD)/../dnvme

CFLAGS += -W -Wall -Werror

$(APP_NAME): $(SOURCES)
	gcc $(CFLAGS) -I$(INCLUDE) $(SOURCES) -o $(APP_NAME)

clean:
	rm -f *.o
	rm -f $(APP_NAME)
