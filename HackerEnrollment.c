#include "HackerEnrollment.h"

typedef struct Applicant{
    int id;
    int totalCredit;
    char* name;
    char* surname;
    char* city;
    char* department;
    unsigned int totalCredits;
    int gpa;
} Applicant;

typedef struct EnrollmentSyetem_t{
    IsraeliQueue queue;
    FILE* courses;
} EnrollmentSyetem_t;

int asciiDiff(Applicant *applicant1, Applicant *applicant2){
    int len1 = strlen(applicant1->name), len2 = strlen(applicant2->name);
    int shorterLength = len1 < len2 ? len1 : len2;
    int sum = 0, temp;
    for(int i = 0; i < shorterLength; i++){
        temp = applicant1->name[i] - applicant2->name[i];
        sum += temp > 0 ? temp : temp * -1;
    }
    return sum;
}

int idDiff(Applicant *applicant1, Applicant *applicant2){
    int diff = applicant1->id - applicant2->id;
    return diff > 0 ? diff : diff * -1;
}

//some hacker function TODO

EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers){
    FriendshipFunction functions[] = {&asciiDiff, &idDiff, NULL};
    EnrollmentSystem system = malloc(sizeof(EnrollmentSystem_t));
    system->queue = IsraeliQueueCreate();
}