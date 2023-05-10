#ifndef PROVIDED_HACKERENROLLMENT_H
#define PROVIDED_HACKERENROLLMENT_H

#include <stdio.h>
#include <stdlib.h>
#include "IsraeliQueue.h"

#define FRIENDSHIP 20
#define ENEMIES 0

#define HACKERFRIEND 20
#define HACKERENEMY -20
#define HACKERNEUTRAL 0

typedef struct EnrollmentSystem_t * EnrollmentSystem;


/**Creates a new struct that contains the students from the file*/
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

/*Reads the queues for an enrollmentsystem from a file*/
EnrollmentSystem readEnrollment(EnrollmentSystem system, FILE* queues);

/*hacks an enrollmentsystem using the given friendship functions*/
void hackEnrollment(EnrollmentSystem system, FILE* out);

/*same as hackenrollment with non case senstive option*/
void hackEnrollmentCaseSenstive(EnrollmentSystem system, FILE* out, bool ignoreCase);

/*destroys an enrolmment system*/
void destroyEnrollment(EnrollmentSystem system);


#endif //PROVIDED_HACKERENROLLMENT_H