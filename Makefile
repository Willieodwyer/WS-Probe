all: multicast listener unicast

listener: listener.c
	gcc listener.c -o listener 

multicast: multicast.c
	gcc multicast.c -o multicast -luuid

unicast: unicast.c
	gcc unicast.c -o unicast -luuid

