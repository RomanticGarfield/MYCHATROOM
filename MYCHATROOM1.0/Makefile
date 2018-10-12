all:poll_server poll_client

poll_server:poll_server.c
	gcc -std=c99 $^ -o $@
poll_client:poll_client.c
	gcc -std=c99 $^ -o $@
	
.PHONY:clean
clean:
	rm -rf poll_server poll_client
