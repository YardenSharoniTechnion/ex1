#include "IsraeliQueue.h"

#include <assert.h>
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
    IsraeliQueue queue = malloc(sizeof(IsraeliQueue_t));
    assert(queue);
    queue->friendshipArray = malloc(ptrlen(friendship) + 1);
    ptrcpy(queue->friendshipArray, friendship);
    queue->dataArray = malloc(sizeof(IsraeliElement));
    assert (queue->dataArray);
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
        queue->dataArray = malloc((elementCount(q->dataArray) + 1)*sizeof(IsraeliElement));
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

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue queue, void* element) {
    if (queue == NULL || element == NULL || queue->dataArray == NULL || queue->friendshipArray == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    IsraeliElement newElement = {element, FRIEND_QUOTA, RIVAL_QUOTA};
    size_t dataSize = elementCount(queue->dataArray) + 1;
    IsraeliElement* oldDataArr = queue->dataArray;
    queue->dataArray = realloc(queue->dataArray, (dataSize + 1) * sizeof(IsraeliElement));
    bool* alreadyBlocked;
    if (queue->dataArray == NULL) {
        queue->dataArray = oldDataArr;
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    queue->dataArray[dataSize].value = NULL;
    int friendshipCount = ptrlen(queue->friendshipArray);
    int elementLocation = dataSize - 1;
    bool done = false;
    IsraeliElement* dataArray = queue->dataArray;
    FriendshipFunction* friendshipArray = queue->friendshipArray;
    int friendshipThreshold = queue->friendshipThreshold;
    bool skipped = false;
    bool* alreadyBlocked = malloc((dataSize + 1) * sizeof(bool));
    for (int i = 0; i < dataSize + 1; i++) {
        alreadyBlocked[i] = false;
    }
    for (int i = 0; i < dataSize; i++) {
        bool friends = false;
        if (dataArray[i].skipCount < 1 || !areFriends(queue, dataArray[i], newElement)) {
            continue;
        }
        for (int j = i; j < dataSize; j++) {
            if (areEnemies(queue, dataArray[j], newElement) && dataArray[j].blockCount > 0) {
                if (!alreadyBlocked[j]) {
                    dataArray[j].blockCount--;
                }
                alreadyBlocked[j] = true;
                continue;
            }
        }
        IsraeliElement* dataTemp = malloc(dataSize - i);
        if (dataTemp == NULL) {
            return ISRAELIQUEUE_ALLOC_FAILED;
        }
        dataArray[i].skipCount--;
        elementArrCopy(dataTemp, dataArray + i);
        dataArray[i] = newElement;
        elementArrCopy(dataArray + i + 1, dataTemp);
        free(dataTemp);
        skipped = true;
        break;
    }
    if (!skipped) {
        dataArray[dataSize - 1] = newElement;
    }
    return ISRAELIQUEUE_SUCCESS;
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
    elementArrCopy(newDataArray, queue->dataArray+1);
    IsraeliElement firstElement = queue->dataArray[0];
    free(queue->dataArray);
    queue->dataArray = newDataArray;
    return firstElement.value;
}

bool IsraeliQueueContains(IsraeliQueue queue , void* element) {
    int elementCount = elementcount(queue->dataArray);
    for (int i = 0; i < elementCount; i++) {
        if (queue->comparisonFunction(queue->dataArray[i].value, element)) {
            return true;
        }
    }
    return false;
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
