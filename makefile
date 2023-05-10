FLAGS = -g -Wall -pedantic-errors


HackEnrollment: IsraeliQueue.o main.o HackerEnrollment.o
	gcc $(FLAGS) main.o HackerEnrollment.o IsraeliQueue.o -o HackEnrollment

IsraeliQueue.o: IsraeliQueue.c IsraeliQueue.h
	gcc $(FLAGS) -c IsraeliQueue.c

HackerEnrollment.o: HackerEnrollment.c HackerEnrollment.h IsraeliQueue.h
	gcc $(FLAGS) -c HackerEnrollment.c

main.o : main.c HackerEnrollment.h
	gcc $(flags) -c main.c HackerEnrollment.h

