#include <bits/stdc++.h>
#include "Queue.h"
#include "Request.h"

Queue::Queue(){
    size = 0;
    head = NULL;
    tail = NULL;
    traversalHead = NULL;
    traversalTail = NULL;
    traversalAnchor = NULL;
}

void Queue::push(long double timestamp, int reqSize, int sentBy, int respSize){
    // Form the request
    Request *incomingRequest = new Request(timestamp, reqSize, sentBy, respSize);
    // Empty Queue
    if(!head){
        head = tail = incomingRequest;
        // Normal request, make it the head
        if(sentBy == -1){
            traversalHead = traversalTail = traversalAnchor = head;
        }
    } else {
        // We already have something
        tail->setNextRequest(incomingRequest);
        incomingRequest->setPrevRequest(tail);
        tail = incomingRequest;
        // If this is a normal request
        if(sentBy == -1){
            // And the first of its kind
            if(!traversalHead){
                traversalHead = traversalTail = traversalAnchor = incomingRequest;
            } else{
                traversalTail->setNextNormalRequest(incomingRequest);
                incomingRequest->setPrevNormalRequest(traversalTail);
                traversalTail = incomingRequest;
            }
        }
    }
    // Increase the size by 1
    size++;
}

Request * Queue::front(){
    /*
    Note: form a copy of this object in your scope before popping if you're gonna need this for later
    */
    return head;
}

void Queue::pop(){
    /*
    Note: this deletes the request object in the Queue. Make sure you have a copy if you need it for later
    */
    if(head){
        if(head == traversalHead){
            traversalHead = traversalHead->getNextNormalRequest();
            traversalHead->setPrevNormalRequest(NULL);
        }
        Request *temp = head->getNextRequest();
        // Destroy the request object
        delete head;
        // Point to the next object
        head = temp;
        head->setPrevRequest(NULL);
        // Decrement size by 1
        size -= 1;
    }
}

void Queue::remove(Request *req){
    /*
    Note: this deletes the request object in the Queue. Make sure you have a copy if you need it for later
    */
    Request *next, *prev, *nextNormal, *prevNormal;
    if(req->getSentBy() == -1){
        // Also a normal request
        nextNormal = req->getNextNormalRequest();
        prevNormal = req->getPrevNormalRequest();
        // Set these values to the correct object
        if(req == traversalHead){
            traversalHead = nextNormal;
        }
        if(req == traversalTail){
            traversalTail = prevNormal;
        }
        if(req == traversalAnchor){
            traversalAnchor = nextNormal;
        }
        // Make the previous and next aware of each other
        if(prevNormal){
            prevNormal->setNextNormalRequest(nextNormal);
        }
        if(nextNormal){
            nextNormal->setPrevNormalRequest(prevNormal);
        }
    }
    next = req->getNextRequest();
    prev = req->getPrevRequest();
    // Set these values to the correct object
    if(req == head){
        head = next;
    }
    if(req == tail){
        tail = prev;
    }
    // Make the previous and next aware of each other
    if(prev){
        prev->setNextRequest(next);
    }
    if(next){
        next->setPrevRequest(prev);
    }
    // Destroy the request object
    delete req;
    // Decrement size by 1
    size -= 1;
}

Request * Queue::yieldNextNormalRequest(bool fromStart=false){
    /*
    Note: Set the fromStart to true only for the first access. For a complete traversal the exit condition in your code has to be a NULL check
    */
    if(fromStart){
        traversalAnchor = head;
    } else {
        if(traversalAnchor){
            traversalAnchor = traversalAnchor->getNextNormalRequest();
        }
    }
    return traversalAnchor;
}

bool Queue::empty(){
    return size==0?true:false;
}