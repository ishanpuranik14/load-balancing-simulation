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
    totalWaitingTime = 0.0;
    avgRespSize = 0.0;
    pendingReqSize = 0;
    processedReqCount = 0;
    pendingReqCount = 0;
    statStartTime = startCollectingAt;
    totalRequestsForwarded = 0;
    requestsForwardedSize = 0;
    totalPendingForwardedRequests = 0;
    pendingForwardedRequestsSize = 0;
    totalForwardedRequestsReceived = 0;
    forwardedRequestsReceivedSize = 0;
}

void Stats::pushProcessedReqQueueForStats(Request cur) {
    if (shouldCollectStats()) {
//        processedReqQueueForStats.push(cur);
        processedReqCount++;
    }
}

long double Stats::getStatStartTime() {
    return statStartTime;
}

long long Stats::getTotalRespSize() const {
    return totalRespSize;
}

void Stats::setTotalRespSize(long long totalRespSize) {
    if (shouldCollectStats()) {
        Stats::totalRespSize = totalRespSize;
    }
}

long long Stats::getTotalReqs() const {
    return totalReqs;
}

void Stats::setTotalReqs(long long totalReqs) {
    if (shouldCollectStats()) {
        Stats::totalReqs = totalReqs;
    }
}

long long Stats::getTotalRespBytesProcessed() const {
    return totalRespBytesProcessed;
}

void Stats::setTotalRespBytesProcessed(long long totalRespBytesProcessed) {
    if (shouldCollectStats()) {
        Stats::totalRespBytesProcessed = totalRespBytesProcessed;
    }
}

long long Stats::getCumulativePendingCount() const {
    return cumulativePendingCount;
}

void Stats::setCumulativePendingCount(long long cumulativePendingCount) {
    if (shouldCollectStats()) {
        Stats::cumulativePendingCount = cumulativePendingCount;
    }
}


long double Stats::getAvgRespSize() const {
    return avgRespSize;
}

void Stats::setAvgRespSize(long double avgRespSize) {
    if (shouldCollectStats()) {
        Stats::avgRespSize = avgRespSize;
    }
}

long double Stats::getUtilization() const {
    return utilization;
}

void Stats::setUtilization(long double utilization) {
    if (shouldCollectStats()) {
        Stats::utilization = utilization;
    }
}

long double Stats::getTotalRespTime() const {
    return totalRespTime;
}

void Stats::setTotalRespTime(long double totalRespTime) {
    if (shouldCollectStats()) {
        Stats::totalRespTime = totalRespTime;
    }
}

long double Stats::getTotalWaitingTime() const {
    return totalWaitingTime;
}

void Stats::setTotalWaitingTime(long double totalWaitingTime) {
    if (shouldCollectStats()) {
        Stats::totalWaitingTime = totalWaitingTime;
    }
}

long double Stats::getTotalBusyTime() const {
    return totalBusyTime;
}

void Stats::setTotalBusyTime(long double totalBusyTime) {
    if (shouldCollectStats()) {
        Stats::totalBusyTime = totalBusyTime;
    }
}

bool Stats::shouldCollectStats() {
    return currentTime >= statStartTime;
}

long double Stats::getAverageServiceRate() {
    return processedReqCount / totalBusyTime;
}

long double Stats::getAvgRespTime() {
    return totalRespTime / processedReqCount;
}

long long Stats::getNumProcessedRequests() {
    return static_cast<long long>(processedReqCount);
}

long double Stats::calculateUtilization(long long alpha) {
    if (currentTime != statStartTime) {
        return (long double) totalRespBytesProcessed / (alpha * (currentTime - statStartTime));
    } else {
        return 0;
    }
}

long long Stats::getPendingRequestSize() {
    return pendingReqSize;
}

void Stats::setPendingRequestSize(long long i) {
    if (shouldCollectStats()) {
        pendingReqSize = i;
    }
}

long long Stats::getPendingReqCount() const {
    return pendingReqCount;
}

void Stats::incrementPendingReqCount() {
    if (shouldCollectStats()) {
        pendingReqCount++;
    }
}

void Stats::decrementPendingReqCount() {
    if (shouldCollectStats()) {
        pendingReqCount--;
    }
}

void Stats::setNumRequestsForwardedToThisServer(long long totalReceived){
    if (shouldCollectStats()) {
         Stats::totalForwardedRequestsReceived = totalReceived;
    }
}

long long Stats::getNumRequestsForwardedToThisServer(){
    return totalForwardedRequestsReceived;
}

void Stats::setTotalSizeForwardedToThisServer(long long totalSizeReceived){
    if (shouldCollectStats()) {
         Stats::forwardedRequestsReceivedSize = totalSizeReceived;
    }
}

long long Stats::getTotalSizeForwardedToThisServer(){
    return forwardedRequestsReceivedSize;
}

void Stats::setPendingForwardedRequestsToThisServer(long long pendingCount){
    if (shouldCollectStats()) {
         Stats::totalPendingForwardedRequests = pendingCount;
    }
}

long long Stats::getPendingForwardedRequestsToThisServer(){
    return totalPendingForwardedRequests;
}

void Stats::setPendingSizeForwardedRequestsToThisServer(long long pendingSize){
    if (shouldCollectStats()) {
         Stats::pendingForwardedRequestsSize = pendingSize;
    }
}

long long Stats::getPendingSizeForwardedRequestsToThisServer(){
    return pendingForwardedRequestsSize;
}

void Stats::setTotalForwardedRequests(long long totalForwarded){
    if (shouldCollectStats()) {
        Stats::totalRequestsForwarded = totalForwarded;
    }
}

long long Stats::getTotalForwardedRequests(){
    return totalRequestsForwarded;
}

void Stats::setTotalForwardedSize(long long totalForwardedSize){
    if (shouldCollectStats()) {
         Stats::requestsForwardedSize = totalForwardedSize;
    }
}

long long Stats::getTotalForwardedSize(){
    return requestsForwardedSize;
}