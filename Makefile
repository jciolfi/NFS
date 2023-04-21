CFLAGS = -Wall -Wextra

all: server client server_test client_test

test: server_test client_test

server: server_handler.h server_handler.c server_main.c common.h common.c
	gcc $(CFLAGS) server_main.c server_handler.c common.c -o fserver -lpthread

client: client_handler.h client_handler.c common.h common.c
	gcc $(CFLAGS) client_main.c client_handler.c common.c -o fget

clean:
	rm -f fget fserver server_test server_test2
	rm -f drive_1/newfile.txt drive_2/newfile.txt journal/server_journal.txt
	rm -f Makefile_copy temp_file.txt
	touch journal/server_journal.txt

server_test: server_handler.c server_test.c common.h common.c
	gcc $(CFLAGS) common.c server_handler.c server_test.c -o server_test

client_test: client_test_single.sh client_test_multi.c client_handler.c common.c
	gcc $(CFLAGS) client_test_multi.c client_handler.c common.c -lpthread -o multi_test
	chmod +x client_test_single.sh