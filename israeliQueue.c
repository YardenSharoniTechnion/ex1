#include "IsraeliQueue.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* value;
    int skipCount;
    int blockCount;
} IsraeliElement;

typedef struct IsraeliQueue_t {
    IsraeliElement* dataArray;
    FriendshipFunction* friendshipArray;
    ComparisonFunction comparisonFunction;
    int friendshipThreshold;
    int rivalryThreshold;
} IsraeliQueue_t;

bool areEnemies(IsraeliQueue queue, IsraeliElement element1, IsraeliElement element2);

bool areFriends(IsraeliQueue queue, IsraeliElement element1, IsraeliElement element2);

double averageFriendship(IsraeliQueue queue, IsraeliElement element1, IsraeliElement element2);

// returns the amount of items in an array of IsraeliElements
int elementCount(IsraeliElement* arr) {
    int i = 0;
    while (arr[i].value != NULL) {
        i++;
    }
    return i;
}

void elementArrCopy(IsraeliElement* destination, IsraeliElement* source) {
    while (destination->value != NULL) {
        *source = *destination;
        source++;
        destination++;
    }
    destination->value = NULL;
    return;
}

// copies a null terminated array of pointers
void ptrcpy(void** destination, void** source) {
    while (*destination != NULL) {
        *source = *destination;
        source++;
        destination++;
    }
    destination = NULL;
    return;
}

// returns the length of a null termianted array of pointers
size_t ptrlen(void** arr) {
    size_t i = 0;
    while (*arr != NULL) {
        i++;
    }
    return i;
}

IsraeliQueue IsraeliQueueCreate(FriendshipFunction* friendship, ComparisonFunction comparison,
                                int friendship_th, int rivalry_th) {
    if (friendship == NULL) {
        return NULL;
    }
    IsraeliQueue queue = malloc(sizeof(IsraeliQueue_t));
    assert(queue);
    queue->friendshipArray = malloc(ptrlen(friendship) + 1);
    ptrcpy(queue->friendshipArray, friendship);
    queue->dataArray = malloc(sizeof(IsraeliElement));
    assert(queue->dataArray);
    queue->dataArray[0].value = NULL;
    queue->comparisonFunction = comparison;
    queue->friendshipThreshold = friendship_th;
    queue->rivalryThreshold = rivalry_th;
    return queue;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue q) {
    IsraeliQueue queue = IsraeliQueueCreate(q->friendshipArray, q->comparisonFunction,
                                            q->friendshipThreshold, q->rivalryThreshold);
    assert(queue);
    if (q->dataArray != NULL) {
        queue->dataArray = malloc((elementCount(q->dataArray) + 1) * sizeof(IsraeliElement));
        elementArrCopy(queue->dataArray, q->dataArray);
    } else {
        queue->dataArray = NULL;
    }
    return queue;
}

void IsraeliQueueDestroy(IsraeliQueue queue) {
    free(queue->dataArray);
    free(queue->friendshipArray);
    free(queue);
    return;
}

// attempts to bump (skip forward) the nth element in a queue, applying the proper effects to other elements.
// if not NULL, places the new position of the element in res
IsraeliQueueError bumpNthElement(IsraeliQueue queue, int n, int* res) {
    if (queue == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }

    size_t elements = elementCount(queue->dataArray) + 1;
    if (n < 0 || n >= elements) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    bool* alreadyBlocked = malloc((elements) * sizeof(bool));
    IsraeliElement* dataArray = queue->dataArray;
    for (int i = 0; i < elements; i++) {
        alreadyBlocked[i] = false;
    }
    for (int i = 0; i < n; i++) {
        if (dataArray[i].skipCount < 1 || !areFriends(queue, dataArray[i], dataArray[n])) {
            continue;
        }
        for (int j = i; j < elements; j++) {
            if (areEnemies(queue, dataArray[j], dataArray[n]) && dataArray[j].blockCount > 0) {
                if (!alreadyBlocked[j]) {
                    dataArray[j].blockCount--;
                }
                alreadyBlocked[j] = true;
                continue;
            }
        }
        if (res != NULL) {
            *res = i;
        }
        dataArray[i].skipCount--;
        IsraeliElement temp = dataArray[i];
        dataArray[i] = dataArray[n];
        for (int j = i + 1; j <= n; j++) {
            IsraeliElement nextTemp = dataArray[j];
            dataArray[j] = temp;
            temp = nextTemp;
        }
        break;
    }
    free(alreadyBlocked);
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue queue, void* element) {
    if (queue == NULL || element == NULL || queue->dataArray == NULL || queue->friendshipArray == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    IsraeliElement newElement = {element, FRIEND_QUOTA, RIVAL_QUOTA};
    size_t dataSize = elementCount(queue->dataArray) + 1;
    IsraeliElement* oldDataArr = queue->dataArray;
    queue->dataArray = realloc(queue->dataArray, (dataSize + 1) * sizeof(IsraeliElement));
    if (queue->dataArray == NULL) {
        queue->dataArray = oldDataArr;
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    queue->dataArray[dataSize].value = NULL;
    int friendshipCount = ptrlen(queue->friendshipArray);
    int elementLocation = dataSize - 1;
    queue->dataArray[dataSize - 1] = newElement;
    bool skipped = false;

    IsraeliQueueError bumpError = bumpNthElement(queue, dataSize - 1, NULL);
    return bumpError;
}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue queue, FriendshipFunction function) {
    if (queue == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    int functionsSize = ptrlen(queue->friendshipArray) + 1;
    FriendshipFunction* oldFriendshipArr = queue->friendshipArray;
    queue->friendshipArray = realloc(queue->friendshipArray, functionsSize + 1);
    if (queue->friendshipArray == NULL) {
        queue->friendshipArray = oldFriendshipArr;
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    queue->friendshipArray[functionsSize] = NULL;
    queue->friendshipArray[functionsSize - 1] = function;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue queue, int newThreshold) {
    if (queue == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    queue->friendshipThreshold = newThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue queue, int newThreshold) {
    if (queue == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    queue->rivalryThreshold = newThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

int IsraeliQueueSize(IsraeliQueue queue) {
    if (queue == NULL) {
        return 0;
    }
    return elementCount(queue);
}

void* IsraeliQueueDequeue(IsraeliQueue queue) {
    if (queue == NULL || queue->dataArray == NULL || elementCount(queue->dataArray) < 1) {
        return NULL;
    }
    IsraeliElement* newDataArray = malloc(elementCount(queue->dataArray));
    assert(newDataArray);
    elementArrCopy(newDataArray, queue->dataArray + 1);
    IsraeliElement firstElement = queue->dataArray[0];
    free(queue->dataArray);
    queue->dataArray = newDataArray;
    return firstElement.value;
}

bool IsraeliQueueContains(IsraeliQueue queue, void* element) {
    int elementCount = elementcount(queue->dataArray);
    for (int i = 0; i < elementCount; i++) {
        if (queue->comparisonFunction(queue->dataArray[i].value, element)) {
            return true;
        }
    }
    return false;
}

IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue queue) {
    size_t elements = elementCount(queue->dataArray);
    bool* alreadyBumped = malloc(elements * sizeof(bool));
    if (alreadyBumped == NULL) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    int i = 0;
    while (i < elementCount) {
        if (alreadyBumped[i]) {
            i++;
            continue;
        }
        int res = -1;
        bool temp = alreadyBumped[i];
        IsraeliQueueError bumpError = bumpNthElement(queue, i, &res);
        if (bumpError != ISRAELIQUEUE_SUCCESS) {
            free(alreadyBumped);
            return bumpError;
        }
        // shift the array to accomadate moving of the element
        for (int j = res + 1; j <= i; j++) {
            bool nextTemp = alreadyBumped[j];
            alreadyBumped[j] = temp;
            temp = nextTemp;
        }
        // and make the element already bumped
        alreadyBumped[res] = true;
        if (res == i) {
            i++;
        }
    }
    free(alreadyBumped);
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueue IsraeliQueueMerge(IsraeliQueue* queueArr, ComparisonFunction function) {
    if (queueArr == NULL) {
        return NULL;
    }
    int queueCount = ptrlen(queueArr);
    FriendshipFunction emptyFunctArray[1] = {NULL};
    int friendshipThreshold = 0;
    int rivalryThreshold = 1;
    for (int i = 0; i < queueCount; i++) {
        friendshipThreshold += queueArr[i]->friendshipThreshold;
        rivalryThreshold *= queueArr[i]->rivalryThreshold;
    }
    friendshipThreshold /= queueCount;
    rivalryThreshold = ceil(pow(rivalryThreshold, ((double)1) / queueCount));
    IsraeliQueue newQueue = IsraeliQueueCreate(emptyFunctArray, function, friendshipThreshold, rivalryThreshold);
    for (int i = 0; i < queueCount; i++) {
        if (queueArr[i]->friendshipArray == NULL) {
            return NULL;
        }
        int functionCount = ptrlen(queueArr[i]->friendshipArray);
        for (int j = 0; j < functionCount; j++) {
            IsraeliQueueAddFriendshipMeasure(newQueue, queueArr[i]->friendshipArray[j]);
        }
    }
    bool done = false;
    while (!done) {
        done = true;
        for (int i = 0; i < queueCount; i++) {
            if (queueArr[i]->dataArray == NULL) {
                return NULL;
            }
            void* element = IsraeliQueueDequeue(queueArr[i]);
            if (element == NULL) {
                continue;
            }
            done = false;
            IsraeliQueueEnqueue(newQueue, element);
        }
    }
    return newQueue;
}
// returns the average friendship of 2 elements in an israeliQueue
double averageFriendship(IsraeliQueue queue, IsraeliElement element1, IsraeliElement element2) {
    int friendshipCount = ptrlen(queue->friendshipArray);
    FriendshipFunction* friendshipArray = queue->friendshipArray;
    int sum = 0;
    for (int i = 0; i < friendshipCount; i++) {
        sum += friendshipArray[i](element1.value, element2.value);
    }
    return sum / friendshipCount;
}

// returns whether the elements given in the queue are friends.
bool areFriends(IsraeliQueue queue, IsraeliElement element1, IsraeliElement element2) {
    int friendshipCount = ptrlen(queue->friendshipArray);
    FriendshipFunction* friendshipArray = queue->friendshipArray;
    for (int i = 0; i < friendshipCount; i++) {
        if (friendshipArray[i](element1.value, element2.value) > queue->friendshipThreshold) {
            return true;
        }
    }
    return false;
}

// returns whether the elements given in the queue are enemies.
bool areEnemies(IsraeliQueue queue, IsraeliElement element1, IsraeliElement element2) {
    if (!areFriends(queue, element1, element2)) {
        if (averageFriendship(queue, element1, element2) < queue->rivalryThreshold) {
            return true;
        }
    }
    return false;
}
