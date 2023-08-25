all:  cmdFtp.o ftp.o
	gcc -Wall -Wextra -I include/ client.c $^ -o client
	
%.o: %.c
	gcc -c $< -Wall -Wextra -I include/ -o $@ 

dirs:
	mkdir obj bin

clean:
	rm *.o client

archive:
