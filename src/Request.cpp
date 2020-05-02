#include <bits/stdc++.h>
#include <spdlog/spdlog.h>
#include "Request.h"
long long reqIdGen = 0;

Request::Request(long double timestamp, int reqSize, int sentBy, int respSize, long long id) {
    if(id == -1){
        reqId = reqIdGen++;
    } else {
        reqId = id;
    }
    spdlog::trace("\tCreated the current request with ID: {}", reqId);
    this->timestamp = timestamp;
    this->reqSize = reqSize;
    this->respSize = respSize;//(int) floor(generate_random_number(200, 500));
    pendingSize = respSize;
    this->sentBy = sentBy;
    forwardingTimestamp = 0;
    finishedTimestamp = 0;
}

double Request::generate_random_number(double low, double high) {
    int random_value = rand();
    return (low + (static_cast<double>(random_value) / (static_cast<double>(RAND_MAX / (high - low)))));
}

long long Request::getReqId() {
    return reqId;
}

int Request::getRespSize() {
    return respSize;
}

int Request::getPendingSize() {
    return pendingSize;
}

long double Request::getTimestamp() {
    return timestamp;
}

void Request::updatePendingSize(long long  bytesProcessed) {
    pendingSize = pendingSize - bytesProcessed;
}

int Request::getSentBy() {
    return sentBy;
}

void Request::updateSentBy(int resentBy) {
    sentBy = resentBy;
}

long double Request::getForwardingTimestamp() {
    return forwardingTimestamp;
}

void Request::updateForwardingTimestamp(long double forwardingTime) {
    this->forwardingTimestamp = forwardingTime;
}

long double Request::getFinishedTimestamp() {
    return finishedTimestamp;
}

void Request::updateFinishedTimestamp(long double finishedTime) {
    this->finishedTimestamp = finishedTime;
}
