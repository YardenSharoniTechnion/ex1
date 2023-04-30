#include "HackerEnrollment.h"

typedef struct {
    int id;
    int totalCredits;
    int gpa;
    char* name;
    char* surname;
    char* city;
    char* department;
} Student;

typedef struct {
    int id;
    int capacity;
    IsraeliQueue queue;
} CourseQueue;

typedef struct {
    CourseQueue* courses;
    int courseCount;
} EnrollmentSystem_t;

EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers) {

}