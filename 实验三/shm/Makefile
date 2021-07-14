all: user1 user2
user1: user1.c
	gcc user1.c -pthread -o user1
user2: user2.c
	gcc user2.c -pthread -o user2
clean:
	rm -rf user1 user2
