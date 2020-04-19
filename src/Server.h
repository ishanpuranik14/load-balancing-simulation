#ifndef LOAD_BALANCING_SIMULATION_SERVER_H
#define LOAD_BALANCING_SIMULATION_SERVER_H

#include <bits/stdc++.h>
#include <spdlog/spdlog.h>
#include "Request.h"
#include "Stats.h"

class Server {
    long long totalFullyProcessedBytes;
    long long alpha;
    int server_no;

    long double avgRespSize, utilization;
    std::queue<Request> reqQueue;

    std::queue<std::pair<int, Request>> deferredRequests;
    Stats stats;

    long long totalReqs;
    long long totalRespSize;
    long long pendingReqSize;

public:

    Server(long long alpha, int server_no, long double startStatCollectionFrom);

    Stats &getStats();

    long long getAlpha();

    long double getUtilization();

    long long getPendingRequestCount();

    long long getPendingRequestSize();

    long double calculateUtilization();

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

    void setPendingRequestSize(long long int i);
};


#endif //LOAD_BALANCING_SIMULATION_SERVER_H