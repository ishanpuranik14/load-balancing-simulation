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
#include "ConfigReader.h"

using namespace std;

char const *serverStatsFileName = "serverStats.csv";
char const *overallStats = "overallStats.csv";
std::ofstream outputFile;

void printStatistics(Server *servers[], int server_count, long double time) {
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

    long long totalPendingRespSize = 0, totalPendingReqsCount = 0, totalBytesProcessed = 0, totalRequestsProcessed = 0, consolidatedCumulativePendingCount = 0;
    long double totalUtilization = 0.0, startTime = 0.0;
    for (int i = 0; i < server_count; i++) {
        Server &server = *servers[i];
        Stats &serverStats = server.getStats();

        long long pendingReqsCount = serverStats.getPendingReqCount();
        long long pendingReqsSize = serverStats.getPendingRequestSize();
        long long bytesProcessed = serverStats.getTotalRespBytesProcessed();
        long long numProcessedRequests = serverStats.getNumProcessedRequests();
        long double serverUtilization = serverStats.calculateUtilization(server.getAlpha());
        long double busyTime = serverStats.getTotalBusyTime();
        long double averageServiceRate = serverStats.getAverageServiceRate();
        long long serverCumulativePendingCount = serverStats.getCumulativePendingCount();
        long double avgRespTime = serverStats.getAvgRespTime();
        long double numPartiallyProcessedRequests = server.getPartiallyProcessedRequestCount();
        long double avgWaitingTime =
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
                     ((long double)serverCumulativePendingCount) / (time - serverStats.getStatStartTime()));
        spdlog::info("\t\t Average Response Time: {}", avgRespTime);
        spdlog::info("\t\t Average Waiting Time: {}", avgWaitingTime);
        outputFile << time << "," << i << "," << pendingReqsCount << "," << pendingReqsSize << ","
                   << numProcessedRequests << "," << bytesProcessed << "," << serverUtilization << "," << busyTime
                   << ","
                   << averageServiceRate << "," << ((long double)serverCumulativePendingCount) / (time - serverStats.getStatStartTime()) << ","
                   << avgRespTime << endl;
    }
    cout << endl;
    outputFile.close();
    outputFile.open(overallStats, std::ios_base::app);
    outputFile << time << "," << totalRequestsProcessed << "," << totalUtilization / double(server_count) << ","
               << totalBytesProcessed << "," << totalPendingReqsCount << "," << totalPendingRespSize << ","
               << ((long double)consolidatedCumulativePendingCount) / (time - startTime) << endl;
    outputFile.close();
    spdlog::info("Cumulative");
    spdlog::info("Total # of requests processed : {}", totalRequestsProcessed);
    spdlog::info("Total size of processed responses : {} bytes", totalBytesProcessed);
    spdlog::info("Total # of pending requests : {}", totalPendingReqsCount);
    spdlog::info("Total pending response size : {} bytes", totalPendingRespSize);
    spdlog::info("Consolidated average number of requests in the system: {} / {} = {}",
                 consolidatedCumulativePendingCount, time, ((long double)consolidatedCumulativePendingCount) / (time - startTime));
}

int main(int argc, char **argv) {
    spdlog::cfg::load_env_levels();
    // Initializations
    ConfigReader* p = ConfigReader::getInstance();

   // parse the configuration file
    p->parseFile("config.txt");

   // Dump map on the console after parsing it
    p->dumpFileValues();

   // Define variables to store the value
    long double maxSimulationTime,snapshotInterval;
    long long alp;
    int respSize,lambda;
   // Update the variable by the value present in the configuration file.
    p->getValue("lambda", lambda);
    p->getValue("alpha", alp );
    p->getValue("maxSimulationTime",maxSimulationTime);
    p->getValue("respSize", respSize);
    p->getValue("snapshotInterval",snapshotInterval);

   // Variables has been updated. Now print it on the console.

    long double snapshotTime = ((snapshotInterval / 100) * maxSimulationTime);
    long double checkTime = snapshotTime;
    // Delete stat file
    if (remove(serverStatsFileName) != 0) {
        spdlog::error("Couldn't delete server stat file");
    }
    if (remove(overallStats) != 0) {
        spdlog::error("Couldn't delete server stat file");
    }
    const int server_count = 1;
    long long alpha[server_count] = {alp};
    Server *servers[server_count];
    spdlog::trace("Simulation parameters");
    spdlog::trace("Simulation time: {}", maxSimulationTime);
    spdlog::trace("Number of server: {}", server_count);
    //Poisson p = Poisson(0.25/4096);
    Uniform u = Uniform(1,lambda);
    for (int i = 0; i < server_count; i++) {
        servers[i] = new Server(alpha[i], i, 0);
    }
    // Iteration
    cout << endl;
    spdlog::trace("----SIMULATION BEGINS----\n\n");
    while (currentTime < maxSimulationTime) {
        int t = 0;
        int nextTimeDelta = (int) u.generate(lambda);
        spdlog::trace("next request in time {}",nextTimeDelta);
        if (currentTime != 0) {
            spdlog::trace("----------------------------------------");
            spdlog::trace("\tTime elapsed {} time units", currentTime);
            spdlog::trace("\tNext request arrives in {} time units", nextTimeDelta);
            Request request = Request(currentTime, 1, -1,respSize);
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
