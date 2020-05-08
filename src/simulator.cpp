#include <cstdio>
#include "Poisson.h"
#include "Uniform.h"
#include "Generator.h"
#include <bits/stdc++.h>
#include <sys/stat.h>
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Server.h"
#include "Clock.h"

using namespace std;

std::ofstream outputFile;
int prev_iteration = 0;

//function to check if a directory for stats of current iteration already exists
int dirExists(const char *path)
{
    struct stat info;
    if(stat( path, &info ) != 0)
        return 0;
    else if(info.st_mode & S_IFDIR)
        return 1;
    else
        return 0;
}

void printStatistics(Server *servers[], int server_count, long double time, long iteration) {
    string folderStart = "results/iteration_";

    //Defining Folder Name for each iteration
    char const *folderName = (folderStart.append(to_string(iteration))).c_str();
    //Creating new Folder for that iteration if it does not exist
    if(dirExists(folderName) == 0){
        mkdir(folderName,0777);
    }
    //Converting folderName to a string type to use it to create the path string for files
    string s(folderName);
    string o(folderName);

    //Creating full path Strings for the server Stats File and Overall stats file (each file is inside the directory for that iteration)
    string serverStats = s.append("/serverStats");
    string overall = o.append("/overallStats");
    char const *serverStatsFileName = serverStats.c_str();
    char const *overallStats = overall.c_str();

    //To check if file already exists for this iteration from a previous program run, deleting if it exists 
    if(iteration!=prev_iteration){
        if (remove(serverStatsFileName) != 0) {
            spdlog::error("Couldn't delete server stat file");
        }
        if (remove(overallStats) != 0) {
            spdlog::error("Couldn't delete server stat file");
        }
    }

    spdlog::info("");
    spdlog::info("----STATISTICS----");
    spdlog::info("");
    spdlog::info("Per server");

    //To check if serverStatsFile for that iteration already exists
    ifstream infile(serverStatsFileName);

    //If it does not exist creating a new file inside the folder for that iteration
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

    //If serverStatsFile exists, appending to that file
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
    prev_iteration = iteration;
}

int main(int argc, char **argv) {
    spdlog::cfg::load_env_levels();
    //Reading command line parameters
    if (argc != 2) {
        spdlog::error("Mising command line parameter for config file name");
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i++) {
        spdlog::info("Command Line Parameters : {}", argv[i]);
    }
    //Reading from csv config file and assigning all parameters
    string configFile(argv[1]);
    ifstream fin((configFile.append(".csv")).c_str());  
    int iteration = 0;
    vector<string> row; 
    string line,word;
    getline(fin,line);
    while (getline(fin,line)) {
        iteration++; 
        row.clear();
        currentTime = 0;
        stringstream s(line); 
        while (getline(s, word, ',')) { 
            row.push_back(word); 
        } 
        int server_count = stoi(row[1]);
        int granularity = stoi(row[6]);
        int lambda = stoi(row[0]);
        int respSize = stoi(row[3])*granularity;
        long double maxSimulationTime = stold(row[4])*granularity;
        long double snapshotInterval = stold(row[5]);
        string dist = row[7];
        long numRequestsForProactive = stoi(row[8]);
        long long alpha[server_count];
        int count = 0;
        row[2] = row[2].substr(1,row[2].length()-2);
        stringstream l(row[2]);
        while(getline(l,word,';')){
            alpha[count] = stoll(word);
            count++;
        }
        // If only 1 alpha was specified
        if(count == 1){
            long long alpha_value = alpha[count-1];
            while(count <= server_count){
                alpha[count] = alpha_value;
                count++;
            }
        }
        long double snapshotTime = ((snapshotInterval / 100) * maxSimulationTime);
        long double checkTime = snapshotTime;
        spdlog::info("Simulation parameters");
        spdlog::info("\tserver count:\t{}",server_count);
        spdlog::info("\tgranularity:\t{}",granularity);
        spdlog::info("\tlambda:\t{}",lambda);
        spdlog::info("\trespSize:\t{}",respSize);
        spdlog::info("\tmaxSimulationTime:\t{}",maxSimulationTime);
        spdlog::info("\tsnapshotInterval:\t{}",snapshotInterval);
        spdlog::info("\tdistrbution:\t{}",dist);
        spdlog::info("\talpha_values:");
        for (int i = 0; i<server_count;i++){
            spdlog::info("\t\t{}",alpha[i]);
        }
        spdlog::info("");
        Server *servers[server_count];
        Poisson p = Poisson(lambda,granularity);
        Uniform u = Uniform(lambda,granularity);
        for (int i = 0; i < server_count; i++) {
            servers[i] = new Server(alpha[i], i, 0);
        }
        // Iteration
        cout << endl;
        spdlog::trace("----SIMULATION BEGINS----\n\n");
        while (currentTime < maxSimulationTime) {
            int t = 0;
            int nextTimeDelta = 0;
            if(dist == "p"){
                //spdlog::info("Using Poisson distribution with lambda: {}\n", double(lambda*1.0/granularity*1.0));
                nextTimeDelta = (int) p.generate();
            }
            else{
                //spdlog::info("Using Uniform distribution with lambda: {} and IAT: {}\n", lambda, granularity);
                nextTimeDelta = (int) u.generate();
            }
            if (currentTime != 0) {
                spdlog::trace("----------------------------------------");
                spdlog::trace("\tTime elapsed {} time units", currentTime);
                spdlog::trace("\tNext request arrives in {} time units", nextTimeDelta);
                spdlog::trace("\tCurrent response size = {}", respSize*granularity);
                int nextServer = rand() % server_count;
                spdlog::trace("\tMapping the request on to server #{}", nextServer);
                (*servers[nextServer]).addRequest(currentTime, respSize*granularity, -1, -1, -1);
                spdlog::trace("number of requests pending for server {}:\t{}", nextServer, (*servers[nextServer]).getPendingRequestCount());
            }
            while ((t++ < nextTimeDelta) && (currentTime < maxSimulationTime)) {
                spdlog::trace("\t\tTime elapsed {} time units", currentTime);
                // Execute policies to forward packets via RDMA
                for (int i = 0; i < server_count; i++) {
                    spdlog::trace("number of requests pending for server {}:\t{}", i, (*servers[i]).getPendingRequestCount());
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
                    printStatistics(servers, server_count, currentTime, iteration);
                    checkTime += snapshotTime;
                }
            }
        }
        spdlog::info("---- ONE ITERATION OF SIMULATION ENDS----");
        spdlog::info("------------------------------------------------------------------------------------------------");
    }
    return 0;
}
