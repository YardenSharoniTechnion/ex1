#include "HackerEnrollment.h"

typedef struct {
    int id;
    IsraeliQueue queue;
} CourseQueue;

typedef struct {
    CourseQueue* courses;
    int courseCount;
} EnrollmentSystem_t;