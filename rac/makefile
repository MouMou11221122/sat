.PHONY:clean all

all: first_process second_process

%:%.c
	gcc -Wall -g -o $@ $< -lpthread -lrt

clean:
	rm -rf first_process second_process
