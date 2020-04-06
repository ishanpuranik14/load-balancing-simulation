#ifndef LOAD_BALANCING_SIMULATION_SERVER_H
#define LOAD_BALANCING_SIMULATION_SERVER_H

#include <bits/stdc++.h>
#include <spdlog/spdlog.h>
#include "Request.h"

class Server {
    long alpha;
    int server_no;
    double avgRespSize, utilization;

    std::queue<Request> reqQueue, processedReqQueue;
    std::queue<std::pair<int, Request>> deferredRequests;

public:
    Server(long alpha, int server_no);

    std::queue<Request> getReqQueue();

    std::queue<Request> getProcessedReqQueue();

    long getAlpha();

    double getTotalWaitingTime();

    double getUtilization();

    double getTotalBusyTime();

    long getPendingRequestCount();

    long getPendingRequestSize();

    long getCumulativePendingCount();

    double calculateUtilization();

    void addRequest(Request request);

    void updatePendingCount();

    long getTotalProcessedBytes();

    double getAverageServiceRate();

    double getAvgRespTime();

    long getNumProcessedRequests();

    bool whenPolicy(int policyNum, int timeDelta, Server *servers[], int server_count);

    std::vector<Request> whatPolicy(int policyNum, int timeDelta, Server *servers[], int server_count);

    int wherePolicy(int policyNum, int timeDelta, Server *servers[], int server_count, Request requestToBeForwarded);

    void removeRequest(Request requestToBeRemoved);

    void forwardRequest(int send_to, Request requestToBeForwarded, Server *servers[], int server_count,
                        bool removeRequestFromQueue = true);

    void forwardDeferredRequests(Server *servers[], int server_count);

    void executeForwardingPipeline(int timeDelta, Server *servers[], int server_count);

    void processData(int timeDelta, Server *servers[], int server_count);

    double getPartiallyProcessedRequestCount();
};


#endif //LOAD_BALANCING_SIMULATION_SERVER_H
