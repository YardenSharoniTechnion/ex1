FLAGS = -g -Wall -pedantic-errors -Werror


HackEnrollment: IsraeliQueue.o main.o HackEnrollment.o
	gcc $(FLAGS) main.o HackEnrollment.o IsraeliQueue.o -o HackEnrollment

IsraeliQueue.o: IsraeliQueue.c IsraeliQueue.h
	gcc $(FLAGS) -c IsraeliQueue.c

HackEnrollment.o: tool/HackEnrollment.c tool/HackEnrollment.h IsraeliQueue.h
	gcc $(FLAGS) -c tool/HackEnrollment.c

main.o : tool/main.c tool/HackEnrollment.h
	gcc $(flags) -c tool/main.c tool/HackEnrollment.h

