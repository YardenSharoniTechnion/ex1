#ifndef PROVIDED_HACKERENROLLMENT_H
#define PROVIDED_HACKERENROLLMENT_H

#include <stdio.h>
#include "IsraeliQueue.h"

typedef struct EnrollmentSyetem_t * EnrollmentSystem;


/**Creates a new struct that contains the students from the file*/
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

/**/
EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

/**/
void hackEnrollment(EnrollmentSystem sys, FILE* out);


#endif //PROVIDED_HACKERENROLLMENT_H