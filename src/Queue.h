#ifndef QUEUE_H
#define QUEUE_H
#include "Request.h"

class Queue{
    long long size;
    Request *head, *tail, *traversalHead, *traversalTail, *traversalAnchor;

    public:
    Queue();

    void push(long double timestamp, int reqSize, int sentBy, int respSize);

    Request* front();

    void pop();

    void remove(Request* req);

    Request* yieldNextNormalRequest(bool fromStart);

    bool empty();
};

#endif //QUEUE_H