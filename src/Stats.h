#ifndef LOAD_BALANCING_SIMULATION_STATS_H
#define LOAD_BALANCING_SIMULATION_STATS_H

#include <bits/stdc++.h>
#include "Request.h"
#include "Server.h"
#include <cstdio>

char const *serverStats = "serverStats.csv";
char const *overallStats = "overallStats.csv";

class Stats {
    long totalRespSize, totalReqs, totalRespBytesProcessed, cumulativePendingCount, totalFullyProcessedBytes;

    double avgRespSize, utilization, totalRespTime, totalWaitingTime;
    double totalBusyTime; // DON'T DELETE. Used for calculating average service rate
    double statStartTime;
    std::ofstream outputFile;
    std::queue<Request> processedReqQueueForStats;

public:
    Stats(double startCollectingAt);

    bool shouldCollectStats();

    void printStatistics(Server *servers[], int server_count, double time);

    long getTotalRespSize() const;

    void setTotalRespSize(long totalRespSize);

    long getTotalReqs() const;

    void setTotalReqs(long totalReqs);

    long getTotalRespBytesProcessed() const;

    void setTotalRespBytesProcessed(long totalRespBytesProcessed);

    long getCumulativePendingCount() const;

    void setCumulativePendingCount(long cumulativePendingCount);

    long getTotalFullyProcessedBytes() const;

    void setTotalFullyProcessedBytes(long totalFullyProcessedBytes);

    double getAvgRespSize() const;

    void setAvgRespSize(double avgRespSize);

    double getUtilization() const;

    void setUtilization(double utilization);

    double getTotalRespTime() const;

    void setTotalRespTime(double totalRespTime);

    double getTotalWaitingTime() const;

    void setTotalWaitingTime(double totalWaitingTime);

    double getTotalBusyTime() const;

    void setTotalBusyTime(double totalBusyTime);
};

#endif //LOAD_BALANCING_SIMULATION_STATS_H
