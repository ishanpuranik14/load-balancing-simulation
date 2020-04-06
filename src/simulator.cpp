#include <cstdio>
#include "Poisson.h"
#include "Uniform.h"
#include "Generator.h"
#include <bits/stdc++.h>
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Request.h"
#include "Stats.h"
#include "Server.h"

using namespace std;

double currentTime = 0;

int main(int argc, char **argv) {
    spdlog::cfg::load_env_levels();
    Stats stats = Stats(0.0);
    // Initializations
    double maxSimulationTime = 10000000;
    double snapshotInterval = 10.0;     // Percentage value
    double snapshotTime = ((snapshotInterval / 100) * maxSimulationTime);
    double checkTime = snapshotTime;
    // Delete stat files
    if (remove(serverStats) != 0) {
        spdlog::error("Couldn't delete server stat file");
    }
    if (remove(overallStats) != 0) {
        spdlog::error("Couldn't delete server stat file");
    }
    const int server_count = 5;
    int alpha[server_count] = {100, 100, 100, 100, 100};
    Server *servers[server_count];
    spdlog::trace("Simulation parameters");
    spdlog::trace("Simulation time: {}", maxSimulationTime);
    spdlog::trace("Number of server: {}", server_count);
    Poisson p = Poisson(0.2);
    for (int i = 0; i < server_count; i++) {
        servers[i] = new Server(alpha[i], i);
    }
    // Iteration
    cout << endl;
    spdlog::trace("----SIMULATION BEGINS----\n\n");
    while (currentTime < maxSimulationTime) {
        int t = 0;
        int nextTimeDelta = (int) p.generate();
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
        }
        while ((t++ < nextTimeDelta) && (currentTime < maxSimulationTime)) {
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
            if (currentTime == checkTime) {
                stats.printStatistics(servers, server_count, currentTime);
                checkTime += snapshotTime;
            }
        }
        if (currentTime == checkTime) {
            stats.printStatistics(servers, server_count, currentTime);
            checkTime += snapshotTime;
        }
    }
    spdlog::trace("----SIMULATION ENDS----");
    cout << endl;
    return 0;
}
