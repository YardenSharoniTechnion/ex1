#include "IsraeliQueue.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
size_t elementCount(IsraeliElement* arr) {
    size_t i = 0;
    while (arr[i].value != NULL) {
        i++;
    }
    return i;
}

void elementArrCopy(IsraeliElement* destination, IsraeliElement* source) {
    int i = 0;
    while (source[i].value != NULL) {
        destination[i] = source[i];
        i++;
    }
    destination[i].value = NULL;
    return;
}

// copies a null terminated array of FriendshipFunctions
void friendshipArrCopy(FriendshipFunction* destination, FriendshipFunction* source) {
    int i = 0;
    while (source[i] != NULL) {
        destination[i] = source[i];
        i++;
    }
    destination[i] = NULL;
    return;
}

// returns the length of a null termianted array of FriendshipFunctions
size_t friendshipArrLength(FriendshipFunction* arr) {
    size_t i = 0;
    while (arr[i] != NULL) {
        i++;
    }
    return i;
}

size_t queueArrLength(IsraeliQueue* arr) {
    size_t i = 0;
    while (arr[i] != NULL) {
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
    queue->friendshipArray = malloc((friendshipArrLength(friendship) + 1) * sizeof(FriendshipFunction));
    assert(queue->friendshipArray);
    friendshipArrCopy(queue->friendshipArray, friendship);
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
    size_t elements = elementCount(queue->dataArray);
    if (n < 0 || n >= elements) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    bool* alreadyBlocked = malloc((elements) * sizeof(bool));
    if (alreadyBlocked == NULL) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    IsraeliElement* dataArray = queue->dataArray;
    for (int i = 0; i < elements; i++) {
        alreadyBlocked[i] = false;
    }
    // special case for when n = 0.
    if (res != NULL) {
        *res = 0;
    }
    bool bumped = false;
    for (int i = 0; i < n; i++) {
        if (dataArray[i].skipCount < 1 || !areFriends(queue, dataArray[i], dataArray[n])) {
            continue;
        }
        bool foundEnemy = false;
        for (int j = i; j < n; j++) {
            if (areEnemies(queue, dataArray[j], dataArray[n]) && dataArray[j].blockCount > 0) {
                if (!alreadyBlocked[j]) {
                    dataArray[j].blockCount--;
                }
                alreadyBlocked[j] = true;
                foundEnemy = true;
                break;
            }
        }
        if (foundEnemy) {
            continue;
        }
        int newSpot = i + 1;
        if (res != NULL) {
            *res = newSpot;
        }
        dataArray[i].skipCount--;
        IsraeliElement temp = dataArray[newSpot];
        dataArray[newSpot] = dataArray[n];
        bumped = true;
        for (int j = newSpot + 1; j <= n; j++) {
            IsraeliElement nextTemp = dataArray[j];
            dataArray[j] = temp;
            temp = nextTemp;
        }
        break;
    }
    if (!bumped && res != NULL) {
        *res = n;
    }
    free(alreadyBlocked);
    return ISRAELIQUEUE_SUCCESS;
}

// moves the nth element in the array to the last place in the array.
IsraeliQueueError moveToLast(IsraeliQueue queue, int n) {
    int elements = elementCount(queue->dataArray);
    IsraeliElement last = queue->dataArray[n];
    IsraeliElement temp = queue->dataArray[elements - 1];
    for (int i = elements - 1; i >= n; i--) {
        IsraeliElement newTemp = queue->dataArray[i];
        queue->dataArray[i] = temp;
        temp = newTemp;
    }
    queue->dataArray[elements - 1] = last;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue queue, void* element) {
    if (queue == NULL || element == NULL || queue->dataArray == NULL || queue->friendshipArray == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    IsraeliElement newElement = {element, FRIEND_QUOTA, RIVAL_QUOTA};
    size_t dataSize = elementCount(queue->dataArray) + 1;
    IsraeliElement* oldDataArr = queue->dataArray;
    IsraeliElement* newDataArr = malloc((dataSize + 1) * sizeof(IsraeliElement));
    queue->dataArray = newDataArr;
    if (queue->dataArray == NULL) {
        queue->dataArray = oldDataArr;
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    elementArrCopy(queue->dataArray, oldDataArr);
    free((void*)oldDataArr);
    queue->dataArray[dataSize].value = NULL;
    queue->dataArray[dataSize - 1] = newElement;
    IsraeliQueueError bumpError = bumpNthElement(queue, dataSize - 1, NULL);
    return bumpError;
}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue queue, FriendshipFunction function) {
    if (queue == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    int functionsSize = friendshipArrLength(queue->friendshipArray) + 1;
    FriendshipFunction* oldFriendshipArr = queue->friendshipArray;
    queue->friendshipArray = malloc((functionsSize + 1) * sizeof(FriendshipFunction));
    if (queue->friendshipArray == NULL) {
        queue->friendshipArray = oldFriendshipArr;
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    friendshipArrCopy(queue->friendshipArray, oldFriendshipArr);
    free(oldFriendshipArr);
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
    return elementCount(queue->dataArray);
}

void* IsraeliQueueDequeue(IsraeliQueue queue) {
    if (queue == NULL || queue->dataArray == NULL || elementCount(queue->dataArray) < 1) {
        return NULL;
    }
    IsraeliElement* newDataArray = malloc(elementCount(queue->dataArray) * sizeof(IsraeliElement));
    assert(newDataArray);
    elementArrCopy(newDataArray, queue->dataArray + 1);
    IsraeliElement firstElement = queue->dataArray[0];
    free(queue->dataArray);
    queue->dataArray = newDataArray;
    return firstElement.value;
}

bool IsraeliQueueContains(IsraeliQueue queue, void* element) {
    int elements = elementCount(queue->dataArray);
    for (int i = 0; i < elements; i++) {
        if (queue->comparisonFunction(queue->dataArray[i].value, element)) {
            return true;
        }
    }
    return false;
}

IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue queue) {
    if (queue == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    size_t elements = elementCount(queue->dataArray);
    int* locations = malloc(elements * sizeof(int));
    if (locations == NULL) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    for (int i = 0; i < elements; i++) {
        locations[i] = i;
    }
    for (int i = elements - 1; i >= 0; i--) {
        int iInArray = -1;
        // the position of this element in the array
        for (int j = 0; j < elements; j++) {
            if (locations[j] == i) {
                iInArray = j;
                break;
            }
        }
        int res = -1;
        IsraeliQueueError lastError = moveToLast(queue, iInArray);
        IsraeliQueueError bumpError = bumpNthElement(queue, elements-1, &res);
        if (bumpError != ISRAELIQUEUE_SUCCESS) {
            free(locations);
            return bumpError;
        }
        int temp = locations[res];
        locations[res] = locations[iInArray];
        // shift locations array
        if (res <= iInArray) {
            for (int j = res + 1; j <= iInArray; j++) {
                int nextTemp = locations[j];
                locations[j] = temp;
                temp = nextTemp;
            }
        }
        else {
            for (int j = res - 1; j >= iInArray; j--) {
                int nextTemp = locations[j];
                locations[j] = temp;
                temp = nextTemp;
            }
        }
    }
    free(locations);
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueue IsraeliQueueMerge(IsraeliQueue* queueArr, ComparisonFunction function) {
    if (queueArr == NULL) {
        return NULL;
    }
    int queueCount = queueArrLength(queueArr);
    FriendshipFunction emptyFunctArray[1] = {NULL};
    int friendshipThreshold = 0;
    int rivalryThreshold = 1;
    for (int i = 0; i < queueCount; i++) {
        friendshipThreshold += queueArr[i]->friendshipThreshold;
        rivalryThreshold *= queueArr[i]->rivalryThreshold;
    }
    friendshipThreshold /= queueCount;
    rivalryThreshold = 2;  // ceil(pow(rivalryThreshold, ((double)1) / queueCount));
    IsraeliQueue newQueue = IsraeliQueueCreate(emptyFunctArray, function, friendshipThreshold, rivalryThreshold);
    for (int i = 0; i < queueCount; i++) {
        if (queueArr[i]->friendshipArray == NULL) {
            return NULL;
        }
        int functionCount = friendshipArrLength(queueArr[i]->friendshipArray);
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
    int friendshipCount = friendshipArrLength(queue->friendshipArray);
    FriendshipFunction* friendshipArray = queue->friendshipArray;
    int sum = 0;
    for (int i = 0; i < friendshipCount; i++) {
        sum += friendshipArray[i](element1.value, element2.value);
    }
    return sum / friendshipCount;
}

// returns whether the elements given in the queue are friends.
bool areFriends(IsraeliQueue queue, IsraeliElement element1, IsraeliElement element2) {
    int friendshipCount = friendshipArrLength(queue->friendshipArray);
    FriendshipFunction* friendshipArray = queue->friendshipArray;
    for (int i = 0; i < friendshipCount; i++) {
        if ((*friendshipArray[i])(element1.value, element2.value) > queue->friendshipThreshold) {
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
