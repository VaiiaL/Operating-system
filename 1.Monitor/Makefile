.PHONY: all clean

all: monitor

monitor: monitor.c
	gcc $^ -lpthread -o $@

clean:
	rm -f monitor