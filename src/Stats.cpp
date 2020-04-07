#include <bits/stdc++.h>
#include "Stats.h"
#include "Clock.h"

using namespace std;

Stats::Stats(double startCollectingAt) {
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

long Stats::getTotalRespSize() const {
    return totalRespSize;
}

void Stats::setTotalRespSize(long totalRespSize) {
    if (shouldCollectStats())
        Stats::totalRespSize = totalRespSize;
}

long Stats::getTotalReqs() const {
    return totalReqs;
}

void Stats::setTotalReqs(long totalReqs) {
    if (shouldCollectStats())
        Stats::totalReqs = totalReqs;
}

long Stats::getTotalRespBytesProcessed() const {
    return totalRespBytesProcessed;
}

void Stats::setTotalRespBytesProcessed(long totalRespBytesProcessed) {
    if (shouldCollectStats())
        Stats::totalRespBytesProcessed = totalRespBytesProcessed;
}

long Stats::getCumulativePendingCount() const {
    return cumulativePendingCount;
}

void Stats::setCumulativePendingCount(long cumulativePendingCount) {
    if (shouldCollectStats())
        Stats::cumulativePendingCount = cumulativePendingCount;
}

long Stats::getTotalFullyProcessedBytes() const {
    return totalFullyProcessedBytes;
}

void Stats::setTotalFullyProcessedBytes(long totalFullyProcessedBytes) {
    if (shouldCollectStats())
        Stats::totalFullyProcessedBytes = totalFullyProcessedBytes;
}

double Stats::getAvgRespSize() const {
    return avgRespSize;
}

void Stats::setAvgRespSize(double avgRespSize) {
    if (shouldCollectStats())
        Stats::avgRespSize = avgRespSize;
}

double Stats::getUtilization() const {
    return utilization;
}

void Stats::setUtilization(double utilization) {
    if (shouldCollectStats())
        Stats::utilization = utilization;
}

double Stats::getTotalRespTime() const {
    return totalRespTime;
}

void Stats::setTotalRespTime(double totalRespTime) {
    if (shouldCollectStats())
        Stats::totalRespTime = totalRespTime;
}

double Stats::getTotalWaitingTime() const {
    return totalWaitingTime;
}

void Stats::setTotalWaitingTime(double totalWaitingTime) {
    if (shouldCollectStats())
        Stats::totalWaitingTime = totalWaitingTime;
}

double Stats::getTotalBusyTime() const {
    return totalBusyTime;
}

void Stats::setTotalBusyTime(double totalBusyTime) {
    if (shouldCollectStats())
        Stats::totalBusyTime = totalBusyTime;
}

bool Stats::shouldCollectStats() {
    return currentTime >= statStartTime;
}

double Stats::getAverageServiceRate() {
    return processedReqQueueForStats.size() / totalBusyTime;
}

double Stats::getAvgRespTime() {
    return totalRespTime / processedReqQueueForStats.size();
}

long Stats::getNumProcessedRequests() {
    return static_cast<long>(processedReqQueueForStats.size());
}

queue<Request> &Stats::getProcessedReqQueueForStats() {
    return processedReqQueueForStats;
}

double Stats::calculateUtilization(long alpha) {
    return (double) totalFullyProcessedBytes / (alpha * currentTime);
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

long Stats::getPendingRequestSize() {
    long numRequests = getPendingReqCount();
    long pendingReqSize = 0;
    while (numRequests--) {
        Request &cur = reqQueue.front();
        pendingReqSize += cur.getPendingSize();
        reqQueue.pop();
        reqQueue.push(cur);
    }
    return pendingReqSize;
}

long Stats::getPendingReqCount() {
    long numRequests = reqQueue.size();
    return numRequests;
}
