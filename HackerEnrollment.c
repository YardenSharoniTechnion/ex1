#include "HackerEnrollment.h"

typedef struct Student {
    int id;
    int totalCredits;
    int gpa;
    char* name;
    char* surname;
    char* city;
    char* department;
} Student;

typedef struct CourseQueue {
    int id;
    int capacity;
    IsraeliQueue queue;
} CourseQueue;

typedef struct EnrollmentSystem_t {
    CourseQueue* courses;
    int courseCount;
    Student* students;
    int studentCount;
} EnrollmentSystem_t;

// reads a string from a file using fscanf, until stopped by either fscanf fail (i.e EOF) or by the stop char.
// dynamically allocates the string to make it the right size.
char* readString(FILE* file, char stop) {
    char* res = malloc(sizeof(char));
    char curr = stop + 1;
    int i = 0;
    while (fscanf(file, ("%c", &curr) > 0)) {
        if (curr == stop) {
            break;
        }
        res[i] = curr;
        char* newRes = realloc(res, (i + 2 * sizeof(char)));
        if (newRes == NULL) {
            free(res);
            return NULL;
        }
        res = newRes;
        res[i + 1] = '\0';
    }
    return res;
}

//Reads a student from a file, returns NULL if failed.
Student* readStudent(FILE* file) {
    Student* res = malloc(sizeof(Student));
    if (fscanf(file, "%d", &res->id) < 1) {
        free(res);
        return NULL;
    }
    if (fscanf(file, "%d", &res->totalCredits) < 1) {
        free(res);
        return NULL;
    }
    if (fscanf(file, "%d", &res->gpa) < 1) {
        free(res);
        return NULL;
    }
    if (fscanf(file, " ") < 1) {
        free(res);
        return NULL;
    }
    res->name = readString(file, ' ');
    if (res->name == NULL) {
        free(res);
        return NULL;
    }
    res->surname = readString(file, ' ');
    if (res->surname == NULL) {
        free(res);
        return NULL;
    }
    res->city = readString(file, ' ');
    if (res->city == NULL) {
        free(res);
        return NULL;
    }
    res->department = readString(file, '\n');
    if (res->department == NULL) {
        free(res);
        return NULL;
    }
    return res;
}

bool getStudentsFromFile(EnrollmentSystem system, FILE* students) {
    system->students = NULL;
    system->studentCount = 0;
    bool done = false;
    while (!done) {
        Student* student = readStudent(students);
        if (student == NULL) {
            done = true; 
            break;
        }
        Student* newStudents = realloc(system->students, (system->studentCount+1)*sizeof(student));
        if (newStudents == NULL) {
            free(system->students);
            return false;;
        }
        system->studentCount++;
        system->students[system->studentCount-1] = *student;
        free(student);
    }
    return true;
}

CourseQueue createCourse(int id, int capacity) {
    CourseQueue res;
    res.id = id;
    res.capacity = capacity;
    res.queue = IsraeliQueueCreate()
}

bool getCoursesFromFile(EnrollmentSystem system, FILE* courses) {
    system->courses = NULL;
    system->studentCount = 0;
    bool done = false;
    while (!done) {
        int id = -1;
        int capacity = -1;
        int scannedAmount = scanf("%d %d", id, capacity);
        if (scannedAmount < 2) {
            done = true; 
            break;
        }
        CourseQueue* newCourses = realloc(system->courses, (system->studentCount+1)*sizeof(CourseQueue));
        if (newCourses == NULL) {
            free(system->courses);
            return false;;
        }
        system->studentCount++;
        system->students[system->studentCount-1];
    }
    return true;
}

EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers) {
    EnrollmentSystem res = malloc(sizeof(EnrollmentSystem_t));
    if (!getStudentsFromFile(res, students)) {
        free(res);
        return NULL;
    }
    if (!getCoursesFromFile(res, courses));
}