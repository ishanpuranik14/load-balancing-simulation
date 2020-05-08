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

    long double avgRespSize;
    std::list<Request> reqQueue;
    std::deque<long double> utilizations, loads;

    std::deque<std::pair<int, std::_List_iterator<Request>>> deferredRequests;
    Stats stats;

    long long totalReqs;
    long long totalRespSize;
    long long pendingReqSize;

public:

    Server(long long alpha, int server_no, long double startStatCollectionFrom);

    Stats &getStats();

    long long getAlpha();

    long double getUtilization(bool historic);

    long long getPendingRequestCount();

    long long getPendingRequestSize();

    long double getServerLoad(bool historic);

    long double calculateUtilization();

    void storeHistoricData(long timeUnits);

    void addRequest(long double timestamp, int respSize, int sentBy, long double forwardingTimestamp, long long id);

    void updatePendingCount();

    bool whenPolicy(int policyNum, int timeDelta, Server *servers[], int server_count, std::map<std::string,int> &policies, std::deque<int> &requestTimeDeltas);

    std::vector<std::_List_iterator<Request>> whatPolicy(int policyNum, int timeDelta, Server *servers[], int server_count);

    int wherePolicy(int policyNum, int timeDelta, Server *servers[], int server_count, Request requestToBeForwarded);

    void removeRequest(std::_List_iterator<Request> requestIter);

    void forwardRequest(int send_to, std::_List_iterator<Request> requestIter, Server *servers[], int server_count,
                        bool removeRequestFromQueue = true);

    void forwardDeferredRequests(Server *servers[], int server_count);

    void executeForwardingPipeline(int timeDelta, Server *servers[], int server_count,std::map<std::string,int> &policies, std::deque<int> &requestTimeDeltas);

    void processData(int timeDelta, Server *servers[], int server_count);

    double getPartiallyProcessedRequestCount();

    void setPendingRequestSize(long long int i);
};


#endif //LOAD_BALANCING_SIMULATION_SERVER_H
