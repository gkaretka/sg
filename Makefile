make:
	gcc main.c communication.h communication.c constants.h objects.h bullets.h bullets.c server.h server.c client.h client.c -I/usr/include/allegro5 -L/usr/lib -lallegro -lpthread -lm -W -Wall

clean:
	rm *.gch a.out
