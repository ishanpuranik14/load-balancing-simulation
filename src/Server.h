#ifndef LOAD_BALANCING_SIMULATION_SERVER_H
#define LOAD_BALANCING_SIMULATION_SERVER_H

#include <bits/stdc++.h>
#include <spdlog/spdlog.h>
#include "Request.h"
#include "Stats.h"

class Server {
    long totalFullyProcessedBytes;
    long alpha;
    int server_no;

    double avgRespSize, utilization;
    std::queue<Request> reqQueue, processedReqQueue;

    std::queue<std::pair<int, Request>> deferredRequests;
    Stats stats;

    long totalReqs;
    long totalRespSize;

public:

    Server(long alpha, int server_no, double startStatCollectionFrom);

    std::queue<Request> getReqQueue();

    Stats &getStats();

    std::queue<Request> getProcessedReqQueue();

    long getAlpha();

    double getUtilization();

    long getPendingRequestCount();

    long getPendingRequestSize();

    double calculateUtilization();

    void addRequest(Request request);

    void updatePendingCount();

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
