#include <cstdio>
#include "Poisson.h"
#include "Uniform.h"
#include <bits/stdc++.h>
#include <sys/stat.h>
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Server.h"
#include "Clock.h"
#include "TracePlayer.h"

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

void printStatistics(Server *servers[], int server_count, long double time, long iteration, const char *configFile, int granularity) {
    long double actualTime = time / granularity;
    string conf(configFile);
    string folderStart = conf.append("/iteration_");
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
    string serverStats = s.append("/serverStats.csv");
    string overall = o.append("/overallStats.csv");
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
                   << "Processed Requests" << "," << "Processed req size" << "," <<"# Requests Forwarded"<< ","
                   <<"Size of Requests Forwarded"<< "," <<"# Forwarded Requests Received"<< ","<<"Size of Forwarded Requests Received"<< ","
                   <<"# Pending Forwarded Requests Received"<< ","<<"Size of Pending Forwarded Requests Received"<< ","<< "Utilization" << ","
                   << "Busy Time" << "," << "Average Service Rate" << "," << "Average # requests in system" << ","
                   << "Average Response Time" << "," << "Average Response Size" << ","<<"Average Response Time"<<","<<"Average Waiting Time"<<","<<"Server Load"<<endl;
        outputFile.close();
        outputFile.open(overallStats);
        outputFile << "Time" << "," << "Total_reqs_processed" << "," << "Avg_utilization" << ","
                   << "total_bytes_processed" << "," << "total_pending_reqs" << "," << "total_pending_respSize" << ","
                   << "total_requests_forwarded" << ","<< "total_size_of_requests_forwarded" << "," << "total_pending_forwarded_requests" << ","
                   << "size_of_pending_forwarded_requests" << "," << "Consolidated average # requests in system" << ","
                   << "Consolidated Average Response Size" << "," << "Total Average Response Time" << ","
                   << "Total Average Waiting Time" << "," << "Total Server Load" << endl;
        outputFile.close();
    }

    //If serverStatsFile exists, appending to that file
    outputFile.open(serverStatsFileName, std::ios_base::app);

    long long totalPendingRespSize = 0, totalPendingReqsCount = 0, totalBytesProcessed = 0, totalRequestsProcessed = 0, consolidatedCumulativePendingCount = 0,
    totalPendingForwardedRequests = 0, totalPendingForwardedRequestsSize = 0, totalForwardedRequestsReceived = 0, totalForwardedRequestsReceivedSize = 0,
    totalRequestsForwarded = 0, totalRequestsForwardedSize = 0;
    long double totalUtilization = 0.0, startTime = 0.0, totalAvgRespSize = 0.0, totalAvgRespTime = 0.0, totalAvgWaitingTime = 0.0, totalServerLoad = 0.0;
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
        long long numRequestsForwarded = serverStats.getTotalForwardedRequests();
        long long sizeOfRequestsForwarded = serverStats.getTotalForwardedSize();
        long long numForwardedRequestsReceived = serverStats.getNumRequestsForwardedToThisServer();
        long long sizeOfForwardedRequestsReceived = serverStats.getTotalSizeForwardedToThisServer();
        long long numPendingForwardedRequests = serverStats.getPendingForwardedRequestsToThisServer();
        long long sizeOfPendingForwardedRequests = serverStats.getPendingSizeForwardedRequestsToThisServer();
        long double avgRespSize = serverStats.getAvgRespSize();
        long double serverLoad = server.getServerLoad();
        // Add to cumulative
        totalPendingReqsCount += pendingReqsCount;
        totalPendingRespSize += pendingReqsSize;
        totalBytesProcessed += bytesProcessed;
        totalRequestsProcessed += numProcessedRequests;
        totalUtilization += serverUtilization;
        consolidatedCumulativePendingCount += serverCumulativePendingCount;
        totalRequestsForwarded += numRequestsForwarded;
        totalRequestsForwardedSize += sizeOfRequestsForwarded;
        totalPendingForwardedRequests += numPendingForwardedRequests;
        totalPendingForwardedRequestsSize += sizeOfPendingForwardedRequests;
        totalForwardedRequestsReceived += numForwardedRequestsReceived;
        totalForwardedRequestsReceivedSize += sizeOfForwardedRequestsReceived;
        totalAvgRespSize += avgRespSize;
        totalAvgRespTime += avgRespTime;
        totalAvgWaitingTime += avgWaitingTime;
        totalServerLoad += serverLoad;
        startTime = serverStats.getStatStartTime();

        // adjust granularity affected stats
        pendingReqsSize /= granularity;
        bytesProcessed /= granularity;
        avgRespTime /= granularity;
        busyTime /= granularity;
        averageServiceRate *= granularity;
        avgWaitingTime /= granularity;
        sizeOfRequestsForwarded /= granularity;
        sizeOfForwardedRequestsReceived /= granularity;
        sizeOfPendingForwardedRequests /= granularity;
        avgRespSize /= granularity;
        serverLoad /= granularity;

        // Print out
        spdlog::info("\tTime: {}", actualTime);
        spdlog::info("\tServer #{}", i);
        spdlog::info("\t\t # of requests processed : {}", numProcessedRequests);
        spdlog::info("\t\t Size of processed responses : {} bytes", bytesProcessed);
        spdlog::info("\t\t # of pending requests : {}", pendingReqsCount);
        spdlog::info("\t\t Size of pending responses : {} bytes", pendingReqsSize);
        spdlog::info("\t\t # of requests forwarded : {}", numRequestsForwarded);
        spdlog::info("\t\t Size of forwarded responses : {} bytes", sizeOfRequestsForwarded);
        spdlog::info("\t\t # of requests forwarded to this server : {}", numForwardedRequestsReceived);
        spdlog::info("\t\t Size of requests forwarded to this server  : {} bytes", sizeOfForwardedRequestsReceived);
        spdlog::info("\t\t # of pending requests forwarded to this server : {}", numPendingForwardedRequests);
        spdlog::info("\t\t Size of pending  responses forwarded to this server : {} bytes", sizeOfPendingForwardedRequests);
        spdlog::info("\t\t Utilization : {} ", serverUtilization);
        spdlog::info("\t\t Busy time: {}", busyTime);
        spdlog::info("\t\t Server Load: {}", serverLoad);
        spdlog::info("\t\t Average Service Rate: {}", averageServiceRate);
        spdlog::info("\t\t Average number of requests in the system: {}",
                     ((long double)serverCumulativePendingCount) / (actualTime - serverStats.getStatStartTime()));
        spdlog::info("\t\t Average Response Time: {}", avgRespTime);
        spdlog::info("\t\t Average Waiting Time: {}", avgWaitingTime);
        spdlog::info("\t\t Average Resp Size: {}", avgRespSize);
        outputFile << actualTime << "," << i << "," << pendingReqsCount << "," << pendingReqsSize << ","
                   << numProcessedRequests << "," << bytesProcessed << "," << numRequestsForwarded << ","
                   << sizeOfRequestsForwarded << "," << numForwardedRequestsReceived << "," << sizeOfForwardedRequestsReceived << ","
                   << numPendingForwardedRequests << "," << sizeOfPendingForwardedRequests << "," << serverUtilization << "," << busyTime << ","
                   << averageServiceRate << "," << ((long double)serverCumulativePendingCount) / (actualTime - serverStats.getStatStartTime()) << ","
                   << avgRespTime << "," << avgRespSize << "," << avgRespTime << "," << avgWaitingTime << "," << serverLoad
                   << endl;
    }
    outputFile.close();

    // adjust granularity affected cumulative stats
    totalPendingRespSize /= granularity;
    totalBytesProcessed /= granularity;
    totalRequestsForwardedSize /= granularity;
    totalPendingForwardedRequestsSize /= granularity;
    totalForwardedRequestsReceivedSize /= granularity;
    totalAvgRespSize /= granularity;
    totalAvgRespTime /= granularity;
    totalAvgWaitingTime /= granularity;
    totalServerLoad /= granularity;

    outputFile.open(overallStats, std::ios_base::app);
    outputFile << actualTime << "," << totalRequestsProcessed << "," << totalUtilization / double(server_count) << ","
               << totalBytesProcessed << "," << totalPendingReqsCount << "," << totalPendingRespSize << ","
               << totalRequestsForwarded << "," << totalRequestsForwardedSize << "," << totalPendingForwardedRequests << ","
               << totalPendingForwardedRequestsSize << "," << ((long double)consolidatedCumulativePendingCount) / (actualTime - startTime) << ","
               << totalAvgRespSize / server_count << "," << totalAvgRespTime / server_count << ","
               << totalAvgWaitingTime / server_count << "," << totalServerLoad << endl;
    outputFile.close();
    spdlog::info("Cumulative");
    spdlog::info("Total # of requests processed : {}", totalRequestsProcessed);
    spdlog::info("Total size of processed responses : {} bytes", totalBytesProcessed);
    spdlog::info("Total # of pending requests : {}", totalPendingReqsCount);
    spdlog::info("Total pending response size : {} bytes", totalPendingRespSize);
    spdlog::info("Total # of requests forwarded : {}", totalRequestsForwarded);
    spdlog::info("Total size of forwarded responses : {} bytes", totalRequestsForwardedSize);
    spdlog::info("Total # of pending forwarded requests: {}", totalPendingForwardedRequests);
    spdlog::info("Total size of pending forwarded responses : {} bytes", totalPendingForwardedRequestsSize);
    spdlog::info("Consolidated average number of requests in the system: {} / {} = {}",
                 consolidatedCumulativePendingCount, actualTime, ((long double)consolidatedCumulativePendingCount) / (actualTime - startTime));
    spdlog::info("Consolidated Average Resp Size: {}", totalAvgRespSize/server_count);
    spdlog::info("Total Server Load: {}", totalServerLoad);
    prev_iteration = iteration;
}

double generateRandomNumber(double low, double high) {
    int random_value = rand();
    return (low + (static_cast<double>(random_value) / (static_cast<double>(RAND_MAX / (high - low)))));
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char **argv) {
    spdlog::cfg::load_env_levels();
    //Reading command line parameters
    if (argc < 2) {
        std::cerr << "Missing command line parameter for config file name" << std::endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i++) {
        std::cout << "Command Line Parameters : " << argv[i] << std::endl;
    }

    // For recording input traces in the first iteration to be used for each subsequent iteration
    bool use_traces = false;
    if(argc == 3){
        use_traces = argv[2][0] == 't';
    }
    TracePlayer tracer = TracePlayer();

    //Reading from csv config file and assigning all parameters
    string results(argv[1]);
    auto split_res = split(results, '/');
    results = split_res[split_res.size() - 1];
    const char *resultsFolder = (results.append("_results")).c_str();
    if(dirExists(resultsFolder)==0){
        mkdir(resultsFolder,0777);
    }

    // write logs to output directory
    try {
        auto filename = results + "/logs.txt";
        remove(filename.c_str());
        cout << filename << endl;
        auto logger = spdlog::basic_logger_mt("x", filename);
        spdlog::set_default_logger(logger);
    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    // Copy config file to output directory
    string configPath(argv[1]);
    string configFile(argv[1]);
    auto split_r = split(configFile, '/');
    configFile = split_r[split_r.size() - 1];
    ifstream fin((configPath.append(".csv")).c_str());
    ofstream configcopy(results + "/" + configFile + ".csv");
    configcopy << fin.rdbuf();
    fin.clear();
    fin.seekg(0);
    configcopy.close();

    int iteration = 0;
    vector<string> row; 
    string line,word;
    getline(fin,line);
    
    // For each iteration in the config file
    while (getline(fin,line)) {
        // Initializations
        srand(static_cast <unsigned> (time(0)));
        iteration++;
        currentTime = 0;
        reqIdGen = 0;
        long long traceIter = reqIdGen;
        deque<RequestSpec> requestSpecs;
        row.clear();
        // Read the parameters from the config file
        stringstream s(line); 
        while (getline(s, word, ',')) { 
            row.push_back(word); 
        } 
        int server_count = stoi(row[1]);
        int granularity = stoi(row[6]);
        double lambda = stod(row[0]);
        row[3] = row[3].substr(1,row[3].length()-2);
        stringstream r(row[3]);
        int c = 0;
        int limits[] ={-1,-1};
        while(getline(r,word,':')){
            limits[c] = stoi(word);
            c++;
        }
        int respSize = -1;
        int maxRespSize = 0; //Max Limit for Random Response Size generation
        int minRespSize = 0; // Min Limit for Random Response Size generation
        if(limits[1]==-1){
            respSize = limits[0]*granularity;
        }
        else{
            maxRespSize = limits[1];
            minRespSize = limits[0];
        }
        long double maxSimulationTime = stold(row[4])*granularity;
        long double snapshotInterval = stold(row[5]);
        string dist = row[7];
        long numRequestsForProactive = stoi(row[8]);
        int what_policy = stoi(row[9]);
        int when_policy = stoi(row[10]);
        int where_policy = stoi(row[11]);
        int k = stoi(row[13]);
        std::map<std::string,int> policies;
        policies["what"] = what_policy;
        policies["when"] = when_policy;
        policies["where"] = what_policy;
        policies["granularity"] = granularity;
        policies["k"] = k;
        long long alpha[server_count];
        int count = 0;
        row[2] = row[2].substr(1,row[2].length()-2);
        stringstream l(row[2]);
        while(getline(l,word,':')){
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
        long double statCollectionStart = stold(row[12])*granularity;
        long double snapshotTime = ((snapshotInterval / 100) * maxSimulationTime);
        long double checkTime = snapshotTime;
        spdlog::info("Simulation parameters");
        spdlog::info("\tusing traces:\t{}",use_traces);
        spdlog::info("\tserver count:\t{}",server_count);
        spdlog::info("\tgranularity:\t{}",granularity);
        spdlog::info("\tlambda:\t{}",lambda/granularity);
        spdlog::info("\trespSize:\t{}",respSize);
        spdlog::info("\tmaxSimulationTime:\t{}",maxSimulationTime);
        spdlog::info("\tsnapshotInterval:\t{}",snapshotInterval);
        spdlog::info("\tdistrbution:\t{}",dist);
        spdlog::info("\twhen policy:\t{}",when_policy);
        spdlog::info("\twhat policy:\t{}",what_policy);
        spdlog::info("\twhere policy:\t{}",where_policy);
        spdlog::info("\talpha_values:");
        for (int i = 0; i<server_count;i++){
            spdlog::info("\t\t{}",alpha[i]);
        }
        spdlog::info("");
        Server *servers[server_count];
        Poisson p = Poisson(lambda,granularity);
        Uniform u = Uniform(lambda,granularity);
        for (int i = 0; i < server_count; i++) {
            servers[i] = new Server(alpha[i], i, statCollectionStart);
        }
        // Iteration
        cout << endl;
        spdlog::trace("----SIMULATION BEGINS----\n\n");
        while (currentTime < maxSimulationTime) {
            int t = 0;
            int nextServer;
            while(requestSpecs.size() <= numRequestsForProactive){
                // Fill in the request specs for proactive policies, based on the distribution
                nextServer = rand() % server_count;
                if (maxRespSize != 0) {//Initialized it to -1 to check later if respSize is one constant value
                    respSize = (int)generateRandomNumber(minRespSize, maxRespSize);
                    // Factor in granularity for the resp size
                    respSize *= granularity;
                }
                if (iteration > 1 && use_traces) {
                    requestSpecs.push_back(tracer.specFor(traceIter++));
                } else if (dist == "p"){
                    RequestSpec spec = {(int)p.generate(), respSize, nextServer};
                    requestSpecs.push_back(spec);
                    if(use_traces){
                        tracer.record(spec);
                    }
                }
                else{
                    RequestSpec spec = {(int)u.generate(), respSize, nextServer};
                    requestSpecs.push_back(spec);
                    if(use_traces){
                        tracer.record(spec);
                    }
                }
            }
            // Get the next request spec
            RequestSpec nextReqSpec = requestSpecs.front();
            int nextTimeDelta = nextReqSpec.timeDelta;
            respSize = nextReqSpec.respSize;
            nextServer = nextReqSpec.server;
            requestSpecs.pop_front();
            if (currentTime != 0) {
                spdlog::trace("----------------------------------------");
                spdlog::trace("\tTime elapsed {} time units", currentTime);
                spdlog::trace("\tNext request arrives in {} time units", nextTimeDelta);
                spdlog::trace("\tCurrent response size = {}", respSize);
                spdlog::trace("\tMapping the request on to server #{}", nextServer);
                (*servers[nextServer]).addRequest(currentTime, respSize, -1, -1, -1);
                spdlog::trace("number of requests pending for server #{}: {}", nextServer, (*servers[nextServer]).getPendingRequestCount());
            }
            while ((t++ < nextTimeDelta) && (currentTime < maxSimulationTime)) {
                spdlog::trace("\t\tTime elapsed {} time units", currentTime);
                if(((long long)currentTime)%granularity == 0){
                    // Execute policies to forward packets via RDMA
                    for (int i = 0; i < server_count; i++) {
                        spdlog::trace("\t\t\tnumber of requests pending for server {}:\t{}", i, (*servers[i]).getPendingRequestCount());
                        (*servers[i]).executeForwardingPipeline(1, servers, server_count, policies, requestSpecs);
                    }
                    // Forward requests
                    for (int i = 0; i < server_count; i++) {
                        (*servers[i]).forwardDeferredRequests(servers, server_count);
                    }
                }
                // Process the requests on each server till the next request comes in
                for (int i = 0; i < server_count; i++) {
                    (*servers[i]).processData(1, servers, server_count);
                    (*servers[i]).updatePendingCount();
                    if(currentTime && ((long long)currentTime)%granularity == 0){
                        (*servers[i]).storeHistoricData(5);
                    }
                }
                currentTime++;
                if (currentTime == checkTime) {
                    printStatistics(servers, server_count, currentTime, iteration,resultsFolder, granularity);
                    checkTime += snapshotTime;
                }
            }
        }
        spdlog::info("---- ITERATION {} OF SIMULATION ENDS----", iteration);
        spdlog::info("------------------------------------------------------------------------------------------------");
        spdlog::info("Tracer recorded {} requests", tracer.traceSize());
    }
    return 0;
}
