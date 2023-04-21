#include "IsraeliQueue.h"
#include <stdlib.h>
#include <assert.h>

IsraeliNode *israeliNodeCreate(void *element);

IsraeliNode *israeliListCLone(IsraeliNode *firstNode);

IsraeliNode *israeliListFromArray(void **array);

void israeliListAppend(IsraeliNode *list, void *data);

// Definition of a node in an israeliQueue.
typedef struct IsraeliNode
{
    void *element;
    struct IsraeliNode *next;
} IsraeliNode;

IsraeliNode *israeliNodeCreate(void *element)
{
    IsraeliNode *node = malloc(sizeof(IsraeliNode));
    assert(node);
    node->element = element;
    node->next = NULL;
    return node;
}

// appends an element to the *end* of a list.
void israeliListAppend(IsraeliNode *list, void *data)
{
    while (list->next != NULL)
    {
        list = list->next;
    }
    list->next = israeliNodeCreate(data);
    return;
}

// Clones a linked list when given it's first node and returns a pointer to the new list.

IsraeliNode *israeliListCLone(IsraeliNode *firstNode)
{
    IsraeliNode *newList = israeliNodeCreate(firstNode->element);
    assert(newList);
    if (firstNode->next != NULL)
    {
        IsraeliNode *newListRecursive = israeliListCLone(firstNode->next);
        newList->next = newListRecursive;
    }
    return newList;
}

// creates an IsraeliList from a null-terminated array.

IsraeliNode *israeliListFromArray(void **array)
{
    IsraeliNode *list = israeliNodeCreate(*array);
    array++;
    while (*array != NULL)
    {
        israeliListAppend(list, *array);
        array++;
    }
    return list;
}

// deallocates all the memory of an israeliList.
void israeliListDestroy(IsraeliNode *list)
{
    if (list->next != NULL)
    {
        israeliListDestroy(list->next);
    }
    free(list);
    return;
}
// We define israeliQueues as linked lists, meaning they're defined by their first element and friendship functions

typedef struct IsraeliQueue_t
{
    IsraeliNode *dataList;
    IsraeliNode *friendshipList;
    ComparisonFunction comparisonFunction;
    int friendshipThreshold;
    int rivalryThreshold;
} IsraeliQueue_t;

IsraeliQueue IsraeliQueueCreate(FriendshipFunction *friendship, ComparisonFunction comparison,
                                int friendship_th, int rivalry_th)
{
    IsraeliQueue queue = malloc(sizeof(IsraeliQueue_t));
    assert(queue);
    queue->friendshipList = israeliListFromArray(friendship);
    queue->comparisonFunction = comparison;
    queue->friendshipThreshold = friendship_th;
    queue->rivalryThreshold = rivalry_th;
    return queue;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue q)
{
    IsraeliQueue queue = malloc(sizeof(IsraeliQueue_t));
    assert(queue);
    queue->dataList = israeliListCLone(q->dataList);
    queue->friendshipList = israeliListCLone(q->friendshipList);
    queue->comparisonFunction = q->comparisonFunction;
    queue->friendshipThreshold = q->friendshipThreshold;
    queue->rivalryThreshold = q->rivalryThreshold;
    return queue;
}

void IsraeliQueueDestroy(IsraeliQueue queue)
{
    israeliListDestroy(queue->friendshipList);
    israeliListDestroy(queue->dataList);
    free(queue);
    return;
}
