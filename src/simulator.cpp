#include <cstdio>
#include "Poisson.h"
#include "Uniform.h"
#include "Generator.h"
#include <bits/stdc++.h>
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Server.h"
#include "Clock.h"

using namespace std;

char const *serverStatsFileName = "serverStats.csv";
char const *overallStats = "overallStats.csv";
std::ofstream outputFile;

void printStatistics(Server *servers[], int server_count, double time) {
    spdlog::info("");
    spdlog::info("----STATISTICS----");
    spdlog::info("");
    spdlog::info("Per server");

    ifstream infile(serverStatsFileName);
    if (!infile.good()) {
        outputFile.open(serverStatsFileName);
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
    outputFile.open(serverStatsFileName, std::ios_base::app);

    long totalPendingRespSize = 0, totalPendingReqsCount = 0, totalBytesProcessed = 0, totalRequestsProcessed = 0, consolidatedCumulativePendingCount = 0;
    double totalUtilization = 0.0, startTime = 0.0;
    for (int i = 0; i < server_count; i++) {
        Server &server = *servers[i];
        Stats &serverStats = server.getStats();

        long pendingReqsCount = serverStats.getPendingReqCount();
        long pendingReqsSize = serverStats.getPendingRequestSize();
        long bytesProcessed = serverStats.getTotalRespBytesProcessed();
        long numProcessedRequests = serverStats.getNumProcessedRequests();
        double serverUtilization = serverStats.calculateUtilization(server.getAlpha());
        double busyTime = serverStats.getTotalBusyTime();
        double averageServiceRate = serverStats.getAverageServiceRate();
        long serverCumulativePendingCount = serverStats.getCumulativePendingCount();
        double avgRespTime = serverStats.getAvgRespTime();
        double numPartiallyProcessedRequests = server.getPartiallyProcessedRequestCount();
        double avgWaitingTime =
                serverStats.getTotalWaitingTime() / (numProcessedRequests + numPartiallyProcessedRequests);
        // Add to cumulative
        totalPendingReqsCount += pendingReqsCount;
        totalPendingRespSize += pendingReqsSize;
        totalBytesProcessed += bytesProcessed;
        totalRequestsProcessed += numProcessedRequests;
        totalUtilization += serverUtilization;
        consolidatedCumulativePendingCount += serverCumulativePendingCount;
        startTime = serverStats.getStatStartTime();
        // Print out
        spdlog::info("\tTime: {}", time);
        spdlog::info("\tServer #{}", i);
        spdlog::info("\t\t # of requests processed : {}", numProcessedRequests);
        spdlog::info("\t\t Size of processed responses : {} bytes", bytesProcessed);
        spdlog::info("\t\t # of pending requests : {}", pendingReqsCount);
        spdlog::info("\t\t Size of pending responses : {} bytes", pendingReqsSize);
        spdlog::info("\t\t Utilization : {} ", serverUtilization);
        spdlog::info("\t\t Busy time: {}", busyTime);
        spdlog::info("\t\t Average Service Rate: {}", averageServiceRate);
        spdlog::info("\t\t Average number of requests in the system: {}",
                     (serverCumulativePendingCount * 1.0) / ((time - serverStats.getStatStartTime()) * 1.0));
        spdlog::info("\t\t Average Response Time: {}", avgRespTime);
        spdlog::info("\t\t Average Waiting Time: {}", avgWaitingTime);
        outputFile << time << "," << i << "," << pendingReqsCount << "," << pendingReqsSize << ","
                   << numProcessedRequests << "," << bytesProcessed << "," << serverUtilization << "," << busyTime
                   << ","
                   << averageServiceRate << "," << (serverCumulativePendingCount * 1.0) / ((time - serverStats.getStatStartTime()) * 1.0) << ","
                   << avgRespTime << endl;
    }
    cout << endl;
    outputFile.close();
    outputFile.open(overallStats, std::ios_base::app);
    outputFile << time << "," << totalRequestsProcessed << "," << totalUtilization / double(server_count) << ","
               << totalBytesProcessed << "," << totalPendingReqsCount << "," << totalPendingRespSize << ","
               << (consolidatedCumulativePendingCount * 1.0) / ((time - startTime) * 1.0) << endl;
    outputFile.close();
    spdlog::info("Cumulative");
    spdlog::info("Total # of requests processed : {}", totalRequestsProcessed);
    spdlog::info("Total size of processed responses : {} bytes", totalBytesProcessed);
    spdlog::info("Total # of pending requests : {}", totalPendingReqsCount);
    spdlog::info("Total pending response size : {} bytes", totalPendingRespSize);
    spdlog::info("Consolidated average number of requests in the system: {} / {} = {}",
                 consolidatedCumulativePendingCount, time, (consolidatedCumulativePendingCount * 1.0) / ((time - startTime) * 1.0));
}

int main(int argc, char **argv) {
    spdlog::cfg::load_env_levels();
    // Initializations
    double maxSimulationTime = 3;
    double snapshotInterval = 10;     // Percentage value
    double snapshotTime = ((snapshotInterval / 100) * maxSimulationTime);
    double checkTime = snapshotTime;
    // Delete stat file
    if (remove(serverStatsFileName) != 0) {
        spdlog::error("Couldn't delete server stat file");
    }
    if (remove(overallStats) != 0) {
        spdlog::error("Couldn't delete server stat file");
    }
    const int server_count = 1;
    int alpha[server_count] = {100};
    Server *servers[server_count];
    spdlog::trace("Simulation parameters");
    spdlog::trace("Simulation time: {}", maxSimulationTime);
    spdlog::trace("Number of server: {}", server_count);
    //Poisson p = Poisson(0.25/4096);
    Uniform p = Uniform(1,5);
    for (int i = 0; i < server_count; i++) {
        servers[i] = new Server(alpha[i], i, 0);
    }
    // Iteration
    cout << endl;
    spdlog::trace("----SIMULATION BEGINS----\n\n");
    while (currentTime < maxSimulationTime) {
        int t = 0;
        int nextTimeDelta = (int) p.generate(5);
        spdlog::trace("next request in time {}",nextTimeDelta);
        if (currentTime != 0) {
            spdlog::trace("----------------------------------------");
            spdlog::trace("\tTime elapsed {} time units", currentTime);
            spdlog::trace("\tNext request arrives in {} time units", nextTimeDelta);
            Request request = Request(currentTime, 1, -1);
            spdlog::trace("\tCreated the current request with ID: {}", request.getReqId());
            spdlog::trace("\tCurrent response size = {}", request.getRespSize());
            int nextServer = rand() % server_count;
            spdlog::trace("\tMapping the request on to server #{}", nextServer);
            (*servers[nextServer]).addRequest(request);
            spdlog::trace("number of requests{}", (*servers[nextServer]).getPendingRequestCount());
        }
        while ((t++ < nextTimeDelta) && (currentTime < maxSimulationTime)) {
            spdlog::trace("number of requests{}", (*servers[0]).getPendingRequestCount());
            spdlog::trace("\t\tTime elapsed {} time units", currentTime);
            // Execute policies to forward packets via RDMA
            for (int i = 0; i < server_count; i++) {
                (*servers[i]).executeForwardingPipeline(1, servers, server_count);
            }
            // Forward requests
            for (int i = 0; i < server_count; i++) {
                (*servers[i]).forwardDeferredRequests(servers, server_count);
            }
            // Process the requests on each server till the next request comes in
            for (int i = 0; i < server_count; i++) {
                (*servers[i]).processData(1, servers, server_count);
                (*servers[i]).updatePendingCount();
            }
            currentTime++;
            if (currentTime == currentTime) {
                printStatistics(servers, server_count, currentTime);
                checkTime += snapshotTime;
            }
        }
    }
    spdlog::trace("----SIMULATION ENDS----");
    cout << endl;
    return 0;
}
