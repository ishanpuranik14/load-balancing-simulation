#include <bits/stdc++.h>
#include "Stats.h"
#include "Clock.h"

using namespace std;

Stats::Stats(long double startCollectingAt) {
    totalRespSize = 0;
    totalReqs = 0;
    totalRespBytesProcessed = 0;
    cumulativePendingCount = 0;
    utilization = 0.0;
    totalBusyTime = 0.0;
    totalRespTime = 0.0;
    totalFullyProcessedBytes = 0.0;
    totalWaitingTime = 0.0;
    avgRespSize = 0.0;
    statStartTime = startCollectingAt;
}

void Stats:: pushProcessedReqQueueForStats(Request cur){
    if(shouldCollectStats()){
        getProcessedReqQueueForStats().push(cur);
    }
}
long double Stats:: getStatStartTime(){
    return statStartTime;
}
long long Stats::getTotalRespSize() const {
    return totalRespSize;
}

void Stats::setTotalRespSize(long long totalRespSize) {
    if (shouldCollectStats())
        Stats::totalRespSize = totalRespSize;
}

long long Stats::getTotalReqs() const {
    return totalReqs;
}

void Stats::setTotalReqs(long long totalReqs) {
    if (shouldCollectStats())
        Stats::totalReqs = totalReqs;
}

long long Stats::getTotalRespBytesProcessed() const {
    return totalRespBytesProcessed;
}

void Stats::setTotalRespBytesProcessed(long long totalRespBytesProcessed) {
    if (shouldCollectStats())
        Stats::totalRespBytesProcessed = totalRespBytesProcessed;
}

long long Stats::getCumulativePendingCount() const {
    return cumulativePendingCount;
}

void Stats::setCumulativePendingCount(long long cumulativePendingCount) {
    if (shouldCollectStats())
        Stats::cumulativePendingCount = cumulativePendingCount;
}

long long Stats::getTotalFullyProcessedBytes() const {
    return totalFullyProcessedBytes;
}

void Stats::setTotalFullyProcessedBytes(long long totalFullyProcessedBytes) {
    if (shouldCollectStats())
        Stats::totalFullyProcessedBytes = totalFullyProcessedBytes;
}

long double Stats::getAvgRespSize() const {
    return avgRespSize;
}

void Stats::setAvgRespSize(long double avgRespSize) {
    if (shouldCollectStats())
        Stats::avgRespSize = avgRespSize;
}

long double Stats::getUtilization() const {
    return utilization;
}

void Stats::setUtilization(long double utilization) {
    if (shouldCollectStats())
        Stats::utilization = utilization;
}

long double Stats::getTotalRespTime() const {
    return totalRespTime;
}

void Stats::setTotalRespTime(long double totalRespTime) {
    if (shouldCollectStats())
        Stats::totalRespTime = totalRespTime;
}

long double Stats::getTotalWaitingTime() const {
    return totalWaitingTime;
}

void Stats::setTotalWaitingTime(long double totalWaitingTime) {
    if (shouldCollectStats())
        Stats::totalWaitingTime = totalWaitingTime;
}

long double Stats::getTotalBusyTime() const {
    return totalBusyTime;
}

void Stats::setTotalBusyTime(long double totalBusyTime) {
    if (shouldCollectStats())
        Stats::totalBusyTime = totalBusyTime;
}

bool Stats::shouldCollectStats() {
    return currentTime >= statStartTime;
}

long double Stats::getAverageServiceRate() {
    return processedReqQueueForStats.size() / totalBusyTime;
}

long double Stats::getAvgRespTime() {
    cout<<" size of processedReqQueueForStats: "<<processedReqQueueForStats.size()<<" | total resp time"<<totalRespTime<<endl;
    return totalRespTime / processedReqQueueForStats.size();
}

long long Stats::getNumProcessedRequests() {
    return static_cast<long long>(processedReqQueueForStats.size());
}

queue<Request> &Stats::getProcessedReqQueueForStats() {
    return processedReqQueueForStats;
}

long double Stats::calculateUtilization(long long alpha) {
    if(currentTime != statStartTime){
        return (long double) totalFullyProcessedBytes / (alpha * (currentTime - statStartTime));
    } else {
        return 0;
    }
}

void Stats::addRequest(Request request) {
    if (shouldCollectStats()) {
        reqQueue.push(request);
    }
}

void Stats::removeRequest(Request requestToBeRemoved) {
    if (shouldCollectStats()) {
        long numRequests = getPendingReqCount();
        while (numRequests--) {
            // Get the request
            Request &cur = reqQueue.front();
            reqQueue.pop();
            // Ignore if this is the one to be removed
            if (cur.getReqId() != requestToBeRemoved.getReqId()) {
                // Add to the back of the queue
                reqQueue.push(cur);
            }
        }
    }
}

long long Stats::getPendingRequestSize() {
    long long numRequests = getPendingReqCount();
    long long pendingReqSize = 0;
    while (numRequests--) {
        Request &cur = reqQueue.front();
        pendingReqSize += cur.getPendingSize();
        reqQueue.pop();
        reqQueue.push(cur);
    }
    return pendingReqSize;
}

long long Stats::getPendingReqCount() {
    long long numRequests = reqQueue.size();
    return numRequests;
}

 void Stats::popRequest(){
     if(shouldCollectStats())
        this->reqQueue.pop();
 }