#define EXPECTED_ARGS 6
#include <stdio.h>
#include <string.h>
#include "HackerEnrollment.h"

int main(int argc, char* argv[]) {
    bool ignoreCase = false;
    FILE* students;
    FILE* courses;
    FILE* hackers;
    FILE* queues;
    FILE* target;
    if (argc < EXPECTED_ARGS) {
        printf("Expected at least %d arguments!\n", EXPECTED_ARGS);
        return 0;
    } else if (argc == EXPECTED_ARGS) {
        ignoreCase = false;
        students = fopen(argv[1], "r");
        courses = fopen(argv[2], "r");
        hackers = fopen(argv[3], "r");
        queues = fopen(argv[4], "r");
        target = fopen(argv[5], "w");
    } else if (argc == EXPECTED_ARGS + 1) {
        ignoreCase = true;
        if (!strcmp(argv[1], "-i")) {
            printf("Unknown flag!\n");
            return 0;
        }
        students = fopen(argv[2], "r");
        courses = fopen(argv[3], "r");
        hackers = fopen(argv[4], "r");
        queues = fopen(argv[5], "r");
        target = fopen(argv[6], "w");
    } 
    if (students == NULL || courses == NULL || hackers == NULL || queues == NULL || target == NULL) {
        printf("Failed to open files!\n");
        return 0;
    }
    EnrollmentSystem system = createEnrollment(students, courses, hackers);
    readEnrollment(system, queues);
    hackEnrollmentCaseSenstive(system, target, ignoreCase);
    destroyEnrollment(system);
    fclose(students);
    fclose(courses);
    fclose(hackers);
    fclose(queues);
    fclose(target);
}