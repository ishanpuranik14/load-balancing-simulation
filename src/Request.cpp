#include <bits/stdc++.h>
#include "Request.h"

long reqIdGen = 0;

Request::Request(int timestamp, int reqSize, int sentBy) {
    reqId = reqIdGen++;
    this->timestamp = timestamp;
    this->reqSize = reqSize;
    respSize = 200;//(int) floor(generate_random_number(200, 500));
    pendingSize = respSize;
    this->sentBy = sentBy;
    forwardingTimestamp = 0;
    finishedTimestamp = 0;
}

double Request::generate_random_number(double low, double high) {
    int random_value = rand();
    return (low + (static_cast<double>(random_value) / (static_cast<double>(RAND_MAX / (high - low)))));
}

long Request::getReqId() {
    return reqId;
}

int Request::getRespSize() {
    return respSize;
}

int Request::getPendingSize() {
    return pendingSize;
}

int Request::getTimestamp() {
    return timestamp;
}

void Request::updatePendingSize(int bytesProcessed) {
    pendingSize = pendingSize - bytesProcessed;
}

int Request::getSentBy() {
    return sentBy;
}

void Request::updateSentBy(int resentBy) {
    sentBy = resentBy;
}

long Request::getForwardingTimestamp() {
    return forwardingTimestamp;
}

void Request::updateForwardingTimestamp(long forwardingTime) {
    this->forwardingTimestamp = forwardingTime;
}

double Request::getFinishedTimestamp() {
    return finishedTimestamp;
}

void Request::updateFinishedTimestamp(double finishedTime) {
    this->finishedTimestamp = finishedTime;
}
