#ifndef LOAD_BALANCING_SIMULATION_STATS_H
#define LOAD_BALANCING_SIMULATION_STATS_H

#include <bits/stdc++.h>
#include "Request.h"
#include <cstdio>

class Stats {
    long long totalRespSize, totalReqs, totalRespBytesProcessed, cumulativePendingCount, pendingReqSize, processedReqCount;
    long double avgRespSize, utilization, totalRespTime, totalWaitingTime;
    long double totalBusyTime; // DON'T DELETE. Used for calculating average service rate
    long double statStartTime;
    std::queue<Request> reqQueue, processedReqQueueForStats;

public:

    Stats(long double startCollectingAt);

    void pushProcessedReqQueueForStats(Request cur);

    long double getStatStartTime();

    bool shouldCollectStats();

    long long getTotalRespSize() const;

    void setTotalRespSize(long long totalRespSize);

    long long getTotalReqs() const;

    void setTotalReqs(long long totalReqs);

    long long getTotalRespBytesProcessed() const;

    void setTotalRespBytesProcessed(long long totalRespBytesProcessed);

    long long getCumulativePendingCount() const;

    void setCumulativePendingCount(long long cumulativePendingCount);

    long double getAvgRespSize() const;

    void setAvgRespSize(long double avgRespSize);

    long double getUtilization() const;

    void setUtilization(long double utilization);

    long double getTotalRespTime() const;

    void setTotalRespTime(long double totalRespTime);

    long double getTotalWaitingTime() const;

    void setTotalWaitingTime(long double totalWaitingTime);

    long double getTotalBusyTime() const;

    void setTotalBusyTime(long double totalBusyTime);

    long double getAverageServiceRate();

    long double getAvgRespTime();

    long long getNumProcessedRequests();

    long double calculateUtilization(long long i);

    void addRequest(Request request);

    void removeRequest(Request request);

    long long getPendingRequestSize();

    long long getPendingReqCount();

    void popRequest();

    void setPendingRequestSize(long long i);
};

#endif //LOAD_BALANCING_SIMULATION_STATS_H
