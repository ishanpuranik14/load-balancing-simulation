#include <bits/stdc++.h>
#include "spdlog/spdlog.h"
#include "Stats.h"

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

void Stats::printStatistics(Server *servers[], int server_count, double time) {
    spdlog::info("");
    spdlog::info("----STATISTICS----");
    spdlog::info("");
    spdlog::info("Per server");

    ifstream infile(serverStats);
    if (!infile.good()) {
        outputFile.open(serverStats);
        outputFile << "Time" << "," << "Server_no" << "," << "Pending_Requests" << "," << "Pending_req_size" << ","
                   << "Processed Requests" << "," << "Processed req size" << "," << "Utilization" << ","
                   << "Busy Time" << "," << "Average Service Rate" << "," << "Average # requests in system" << ","
                   << "Average Response Time" << endl;
        outputFile.close();
        outputFile.open(overallStats);
        outputFile << "Time" << "," << "Total_reqs_processed" << "," << "Avg_utilization" << ","
                   << "total_bytes_processed" << "," << "total_pending_reqs" << "," << "total_pending_respSize"
                   << "," << "Consolidated average # requests in system" << endl;
        outputFile.close();
    }
    outputFile.open(serverStats, std::ios_base::app);

    long totalPendingRespSize = 0, totalPendingReqsCount = 0, totalBytesProcessed = 0, totalRequestsProcessed = 0, consolidatedCumulativePendingCount = 0;
    double totalUtilization = 0.0;
    for (int i = 0; i < server_count; i++) {
        // Get the data
        long pendingReqsCount = (*servers[i]).getPendingRequestCount();
        long pendingReqsSize = (*servers[i]).getPendingRequestSize();
        long bytesProcessed = (*servers[i]).getTotalProcessedBytes();
        long numProcessedRequests = (*servers[i]).getNumProcessedRequests();
        double utilization = (*servers[i]).calculateUtilization();
        double busyTime = (*servers[i]).getTotalBusyTime();
        double averageServiceRate = (*servers[i]).getAverageServiceRate();
        long cumulativePendingCount = (*servers[i]).getCumulativePendingCount();
        double avgRespTime = (*servers[i]).getAvgRespTime();
        double numPartiallyProcessedRequests = (*servers[i]).getPartiallyProcessedRequestCount();
        double avgWaitingTime =
                (*servers[i]).getTotalWaitingTime() / (numProcessedRequests + numPartiallyProcessedRequests);
        // Add to cumulative
        totalPendingReqsCount += pendingReqsCount;
        totalPendingRespSize += pendingReqsSize;
        totalBytesProcessed += bytesProcessed;
        totalRequestsProcessed += numProcessedRequests;
        totalUtilization += utilization;
        consolidatedCumulativePendingCount += cumulativePendingCount;
        // Print out
        spdlog::info("\tTime: {}", time);
        spdlog::info("\tServer #{}", i);
        spdlog::info("\t\t # of requests processed : {}", numProcessedRequests);
        spdlog::info("\t\t Size of processed responses : {} bytes", bytesProcessed);
        spdlog::info("\t\t # of pending requests : {}", pendingReqsCount);
        spdlog::info("\t\t Size of pending responses : {} bytes", pendingReqsSize);
        spdlog::info("\t\t Utilization : {} ", utilization);
        spdlog::info("\t\t Busy time: {}", busyTime);
        spdlog::info("\t\t Average Service Rate: {}", averageServiceRate);
        spdlog::info("\t\t Average number of requests in the system: {}",
                     (cumulativePendingCount * 1.0) / (time * 1.0));
        spdlog::info("\t\t Average Response Time: {}", avgRespTime);
        spdlog::info("\t\t Average Waiting Time: {}", avgWaitingTime);
        outputFile << time << "," << i << "," << pendingReqsCount << "," << pendingReqsSize << ","
                   << numProcessedRequests << "," << bytesProcessed << "," << utilization << "," << busyTime << ","
                   << averageServiceRate << "," << (cumulativePendingCount * 1.0) / (time * 1.0) << ","
                   << avgRespTime << endl;
    }
    cout << endl;
    outputFile.close();
    outputFile.open(overallStats, std::ios_base::app);
    outputFile << time << "," << totalRequestsProcessed << "," << totalUtilization / double(server_count) << ","
               << totalBytesProcessed << "," << totalPendingReqsCount << "," << totalPendingRespSize << ","
               << (consolidatedCumulativePendingCount * 1.0) / (time * 1.0) << endl;
    outputFile.close();
    spdlog::info("Cumulative");
    spdlog::info("Total # of requests processed : {}", totalRequestsProcessed);
    spdlog::info("Total size of processed responses : {} bytes", totalBytesProcessed);
    spdlog::info("Total # of pending requests : {}", totalPendingReqsCount);
    spdlog::info("Total pending response size : {} bytes", totalPendingRespSize);
    spdlog::info("Consolidated average number of requests in the system: {} / {} = {}",
                 consolidatedCumulativePendingCount, time, (consolidatedCumulativePendingCount * 1.0) / time);
}

long Stats::getTotalRespSize() const {
    return totalRespSize;
}

void Stats::setTotalRespSize(long totalRespSize) {
    Stats::totalRespSize = totalRespSize;
}

long Stats::getTotalReqs() const {
    return totalReqs;
}

void Stats::setTotalReqs(long totalReqs) {
    Stats::totalReqs = totalReqs;
}

long Stats::getTotalRespBytesProcessed() const {
    return totalRespBytesProcessed;
}

void Stats::setTotalRespBytesProcessed(long totalRespBytesProcessed) {
    Stats::totalRespBytesProcessed = totalRespBytesProcessed;
}

long Stats::getCumulativePendingCount() const {
    return cumulativePendingCount;
}

void Stats::setCumulativePendingCount(long cumulativePendingCount) {
    Stats::cumulativePendingCount = cumulativePendingCount;
}

long Stats::getTotalFullyProcessedBytes() const {
    return totalFullyProcessedBytes;
}

void Stats::setTotalFullyProcessedBytes(long totalFullyProcessedBytes) {
    Stats::totalFullyProcessedBytes = totalFullyProcessedBytes;
}

double Stats::getAvgRespSize() const {
    return avgRespSize;
}

void Stats::setAvgRespSize(double avgRespSize) {
    Stats::avgRespSize = avgRespSize;
}

double Stats::getUtilization() const {
    return utilization;
}

void Stats::setUtilization(double utilization) {
    Stats::utilization = utilization;
}

double Stats::getTotalRespTime() const {
    return totalRespTime;
}

void Stats::setTotalRespTime(double totalRespTime) {
    Stats::totalRespTime = totalRespTime;
}

double Stats::getTotalWaitingTime() const {
    return totalWaitingTime;
}

void Stats::setTotalWaitingTime(double totalWaitingTime) {
    Stats::totalWaitingTime = totalWaitingTime;
}

double Stats::getTotalBusyTime() const {
    return totalBusyTime;
}

void Stats::setTotalBusyTime(double totalBusyTime) {
    Stats::totalBusyTime = totalBusyTime;
}

bool Stats::shouldCollectStats() {
    return currentTime >= statStartTime;
}
