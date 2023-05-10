#include "HackerEnrollment.h"

#include <assert.h>

typedef struct Hacker Hacker;

typedef struct Student {
    int id;
    int totalCredits;
    int gpa;
    char* name;
    char* surname;
    char* city;
    char* department;
    bool isHacker;
    Hacker* hacker;
    bool alreadyPrinted;
} Student;

struct Hacker {
    int id;
    int* friendIds;
    int friendCount;
    int* enemyIds;
    int enemyCount;
    int* courses;
    int courseCount;
    Student* student;
};

typedef struct CourseQueue {
    bool initialized;
    int id;
    int capacity;
    IsraeliQueue queue;
} CourseQueue;

typedef struct EnrollmentSystem_t {
    CourseQueue* courses;
    int courseCount;
    Student* students;
    int studentCount;
    Hacker** hackers;
    int hackerCount;
} EnrollmentSystem_t;

int min(int a, int b) {
    if (a > b) {
        return b;
    }
    return a;
}

// reads a string from a file using fscanf, until stopped by either fscanf fail (i.e EOF) or by the stop char.
// dynamically allocates the string to make it the right size.
char* readString(FILE* file, char stop) {
    char* res = malloc(sizeof(char));
    char curr = stop + 1;
    int i = 0;
    while (fscanf(file, "%c", &curr) > 0) {
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

// Reads a student from a file, returns NULL if failed.
Student* readStudent(FILE* file) {
    Student* res = malloc(sizeof(Student));
    res->alreadyPrinted = false;
    res->isHacker = false;
    res->hacker = NULL;
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
    if (fscanf(file, " ") < 0) {
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
        Student* newStudents = realloc(system->students, (system->studentCount + 1) * sizeof(Student));
        if (newStudents == NULL) {
            free(system->students);
            return false;
        }
        system->students = newStudents;
        system->studentCount++;
        system->students[system->studentCount - 1] = *student;
        free(student);
    }
    return true;
}

int idCompare(void* student1, void* student2) {
    Student* student1new = student1;
    Student* student2new = student2;
    return student1new->id == student2new->id;
}

int asciiDiff(void* student1, void* student2) {
    Student* student1new = student1;
    Student* student2new = student2;
    int sum = 0;
    for (int i = 0; student1new->name[i] && student2new->name[i]; i++) {
        sum += abs(student1new->name[i] - student2new->name[i]);
    }
    for (int i = 0; student1new->surname[i] && student2new->surname[i]; i++) {
        sum += abs(student1new->surname[i] - student2new->surname[i]);
    }
    return abs(sum);
}

char toLowerCase(char in) {
    if ('A' <= in && in <= 'Z') {
        in += 'a' - 'A';
    }
    return in;
}

int asciiDiffIgnoreCase(void* student1, void* student2) {
    Student* student1new = student1;
    Student* student2new = student2;
    int sum = 0;
    for (int i = 0; student1new->name[i] && student2new->name[i]; i++) {
        sum += abs(toLowerCase(student1new->name[i]) - toLowerCase(student2new->name[i]));
    }
    for (int i = 0; student1new->surname[i] && student2new->surname[i]; i++) {
        sum += abs(toLowerCase(student1new->surname[i]) - toLowerCase(student2new->surname[i]));
    }
    return abs(sum);
}

int idDiff(void* student1, void* student2) {
    Student* student1new = student1;
    Student* student2new = student2;
    return abs(student1new->id - student2new->id);
}

int hackerDiff(void* student1, void* student2) {
    Student* student1Real = student1;
    Student* student2Real = student2;
    Student* hackerStudent = NULL;
    Student* otherStudent = NULL;
    if (student1Real->isHacker) {
        hackerStudent = student1Real;
        otherStudent = student2Real;
    } else if (student2Real->isHacker) {
        hackerStudent = student2Real;
        otherStudent = student1Real;
    } else {
        return HACKERNEUTRAL;
    }
    for (int i = 0; i < hackerStudent->hacker->friendCount; i++) {
        if (hackerStudent->hacker->friendIds[i]) {
            return HACKERFRIEND;
        }
    }
    for (int i = 0; i < hackerStudent->hacker->enemyCount; i++) {
        if (hackerStudent->hacker->enemyIds[i]) {
            return HACKERENEMY;
        }
    }
    return HACKERNEUTRAL;
}

CourseQueue createCourse(int id, int capacity) {
    CourseQueue res;
    res.id = id;
    res.capacity = capacity;
    res.initialized = false;
    FriendshipFunction functions[] = {NULL};
    res.queue = IsraeliQueueCreate(functions, idCompare, FRIENDSHIP, ENEMIES);
    return res;
}

bool getCoursesFromFile(EnrollmentSystem system, FILE* courses) {
    system->courses = NULL;
    system->courseCount = 0;
    bool done = false;
    while (!done) {
        int id = -1;
        int capacity = -1;
        int scannedAmount = fscanf(courses, "%d %d", &id, &capacity);
        if (scannedAmount < 2) {
            done = true;
            break;
        }
        CourseQueue* newCourses = realloc(system->courses, (system->courseCount + 1) * sizeof(CourseQueue));
        if (newCourses == NULL) {
            free(system->courses);
            return false;
        }
        system->courses = newCourses;
        system->courseCount++;
        system->courses[system->courseCount - 1] = createCourse(id, capacity);
        if (system->courses[system->courseCount - 1].queue == NULL) {
            for (int i = 0; i < system->courseCount - 1; i++) {
                IsraeliQueueDestroy(system->courses[i].queue);
                return false;
            }
        }
    }
    return true;
}

int* getInts(FILE* file, int* intCountOut, char finish) {
    char temp = 0;
    int* arr = NULL;
    int size = 0;
    while ((temp) != finish) {
        int friendId = 0;
        if (fscanf(file, "%d", &friendId) < 1) {
            return NULL;
        }
        int* newFriendIds = realloc(arr, sizeof(int) * (size + 1));
        if (newFriendIds == NULL) {
            free(arr);
            return NULL;
        }
        arr = newFriendIds;
        size++;
        arr[size - 1] = friendId;
        fscanf(file, "%c", &temp);
    }
    if (intCountOut != NULL) {
        *intCountOut = size;
    }
    return arr;
}

void destroyHacker(Hacker hacker) {
    free(hacker.courses);
    free(hacker.friendIds);
    free(hacker.enemyIds);
}

void destroyPHacker(Hacker* hacker) {
    destroyHacker(*hacker);
    free(hacker);
}

Hacker* getHackerFromFile(FILE* file) {
    Hacker* res = malloc(sizeof(Hacker));
    if (fscanf(file, "%d", &res->id) < 0) {
        free(res);
        return NULL;
    }
    res->courses = getInts(file, &res->courseCount, '\n');
    if (res->courses == NULL) {
        destroyHacker(*res);
        free(res);
        return NULL;
    }
    res->friendIds = getInts(file, &res->friendCount, '\n');
    if (res->friendIds == NULL) {
        destroyHacker(*res);
        free(res);
        return NULL;
    }
    res->enemyIds = getInts(file, &res->enemyCount, '\n');
    if (res->enemyIds == NULL) {
        destroyHacker(*res);
        free(res);
        return NULL;
    }
    return res;
}

Hacker* readHacker(FILE* input) {
    Hacker* res = malloc(sizeof(Hacker));
    if (fscanf(input, "%d", &res->id) < 1) {
        free(res);
        return NULL;
    }
    int courseCount, friendCount, enemyCount;
    int sum = 0;
    res->courses = getInts(input, &courseCount, '\n');
    res->friendIds = getInts(input, &friendCount, '\n');
    res->enemyIds = getInts(input, &enemyCount, '\n');
    res->courseCount = courseCount;
    res->friendCount = friendCount;
    res->enemyCount = enemyCount;
    if (res->courses == NULL || res->friendIds == NULL || res->enemyIds == NULL) {
        free(res);
        return NULL;
    }
    return res;
}

bool getHackersFromFile(EnrollmentSystem system, FILE* hackers) {
    bool done = false;
    system->hackers = NULL;
    system->hackerCount = 0;
    while (!done) {
        Hacker* hacker = readHacker(hackers);
        if (hacker == NULL) {
            done = true;
            break;
        }
        int hackerId = hacker->id;
        for (int i = 0; i < system->studentCount; i++) {
            if (system->students[i].id == hackerId) {
                system->students[i].isHacker = true;
                system->students[i].hacker = hacker;
                hacker->student = &system->students[i];
                break;
            }
        }
        Hacker** newHackerArr = realloc(system->hackers, (system->hackerCount + 1) * sizeof(Hacker*));
        if (newHackerArr == NULL) {
            return false;
        }
        system->hackerCount++;
        system->hackers = newHackerArr;
        system->hackers[system->hackerCount - 1] = hacker;
    }
    return true;
}

void destroyStudent(Student student) {
    free(student.name);
    free(student.surname);
    free(student.city);
    free(student.department);
}

void destroyStudents(EnrollmentSystem system) {
    for (int i = 0; i < system->studentCount; i++) {
        destroyStudent(system->students[i]);
    }
    free(system->students);
}

void destroyCourses(EnrollmentSystem system) {
    for (int i = 0; i < system->courseCount; i++) {
        IsraeliQueueDestroy(system->courses[i].queue);
    }
    free(system->courses);
}

EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers) {
    EnrollmentSystem res = malloc(sizeof(EnrollmentSystem_t));
    if (!getStudentsFromFile(res, students)) {
        free(res);
        return NULL;
    }
    if (!getCoursesFromFile(res, courses)) {
        destroyStudents(res);
        free(res);
        return NULL;
    }
    if (!getHackersFromFile(res, hackers)) {
        destroyStudents(res);
        destroyCourses(res);
        free(res);
        return NULL;
    }
    return res;
}

EnrollmentSystem readEnrollment(EnrollmentSystem system, FILE* queues) {
    bool done = false;
    while (!done) {
        int arrLen = 0;
        int* arr = getInts(queues, &arrLen, '\n');
        if (arrLen < 2) {
            done = true;
            return system;
        }
        int courseNumber = arr[0];
        int courseIndex = -1;
        for (int i = 0; i < system->courseCount; i++) {
            if (system->courses[i].id == courseNumber) {
                courseIndex = i;
                system->courses[i].initialized = true;
                break;
            }
        }
        if (courseIndex == -1) {
            continue;
        }
        for (int i = 1; i < arrLen; i++) {
            for (int j = 0; j < system->studentCount; j++) {
                if (system->students[j].id == arr[i]) {
                    IsraeliQueueEnqueue(system->courses[courseIndex].queue, (&(system->students[j])));
                }
            }
        }
        free(arr);
    }
    return system;
}

// returns a pointer to a courseQueue of the given id
CourseQueue* findQueue(EnrollmentSystem system, int id) {
    for (int i = 0; i < system->courseCount; i++) {
        if (system->courses[i].id == id) {
            return &system->courses[i];
        }
    }
    return NULL;
}

/*EnrollmentSystem cloneSystem(EnrollmentSystem system) {
    EnrollmentSystem newSystem = malloc(sizeof(EnrollmentSystem_t));
    newSystem->courseCount = system->courseCount;
    newSystem->hackerCount = system->hackerCount;
    newSystem->studentCount = system->studentCount;

}*/

void hackEnrollment(EnrollmentSystem system, FILE* out) {
    hackEnrollmentCaseSenstive(system, out, false);
}

void hackEnrollmentCaseSenstive(EnrollmentSystem system, FILE* out, bool ignoreCase) {
    for (int i = 0; i < system->courseCount; i++) {
        if (!ignoreCase) {
            IsraeliQueueAddFriendshipMeasure(system->courses[i].queue, asciiDiff);
        } else {
            IsraeliQueueAddFriendshipMeasure(system->courses[i].queue, asciiDiffIgnoreCase);
        }
        IsraeliQueueAddFriendshipMeasure(system->courses[i].queue, idDiff);
        IsraeliQueueAddFriendshipMeasure(system->courses[i].queue, hackerDiff);
    }
    for (int i = 0; i < system->hackerCount; i++) {
        Hacker* hacker = system->hackers[i];
        for (int j = 0; j < hacker->courseCount; j++) {
            CourseQueue* currCourse = findQueue(system, hacker->courses[j]);
            if (!currCourse->initialized) {
                continue;
            }
            assert(currCourse);
            IsraeliQueueEnqueue(currCourse->queue, hacker->student);
            bool successfullyAdded = false;
            IsraeliQueue tempQueue = IsraeliQueueClone(currCourse->queue);
            int courseSize = min(currCourse->capacity, IsraeliQueueSize(tempQueue));
            for (int k = 0; k < courseSize; k++) {
                if (((Student*)IsraeliQueueDequeue(tempQueue))->id == hacker->student->id) {
                    successfullyAdded = true;
                    break;
                }
            }
            IsraeliQueueDestroy(tempQueue);
            if (!successfullyAdded) {
                fprintf(out, "Cannot satisfy constraints for %d", hacker->id);
                return;
            }
        }
    }
    for (int i = 0; i < system->courseCount; i++) {
        CourseQueue* currCourse = &system->courses[i];
        if (!currCourse->initialized) {
            continue;
        }
        IsraeliQueue tempQueue = IsraeliQueueClone(currCourse->queue);
        fprintf(out, "%d", currCourse->id);
        for (int j = 0; j < system->studentCount; j++) {
            system->students[j].alreadyPrinted = false;
        }
        int courseSize = min(currCourse->capacity, IsraeliQueueSize(tempQueue));
        for (int k = 0; k < courseSize; k++) {
            Student* student = (Student*)IsraeliQueueDequeue(tempQueue);
            if (!student->alreadyPrinted) {
                fprintf(out, " %d", student->id);
            }
            student->alreadyPrinted = true;
        }
        fprintf(out, "\n");
        IsraeliQueueDestroy(tempQueue);
    }
}


void destroyEnrollment(EnrollmentSystem system) {
    for (int i = 0; i < system->hackerCount; i++) {
        Hacker* hacker = system->hackers[i];
        destroyPHacker(hacker);
    }
    free(system->hackers);
    for (int i = 0; i < system->studentCount; i++) {
        Student student = system->students[i];
        destroyStudent(student);
    }
    free(system->students);
    for (int i = 0; i < system->courseCount; i++) {
        IsraeliQueueDestroy(system->courses[i].queue);
    }
    free(system->courses);
    free(system);
}