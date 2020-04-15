#include <bits/stdc++.h>
#include "Uniform.h"
#include "spdlog/spdlog.h"

Uniform::Uniform(double mean) : Generator() {
    this->mean = mean;
    spdlog::info("Using Uniform distribution with lambda: {}\n", mean);
}

Uniform::Uniform(double mean, int currentRequests){
    this->mean = mean;
    this-> currentRequests = currentRequests;
    spdlog::info("Using Uniform distribution with lambda: {}\n", currentRequests);
}

double Uniform::generate() {
    return this->mean;
}

double Uniform::generate(int numRequests){
    if(this->currentRequests>0 && this->currentRequests!=numRequests){
        this->currentRequests-=1;
        std::cout <<"from uniform"<< 0<<"\n";
        return 0;
    }
    else{
        if(this->currentRequests==numRequests){
            this->currentRequests-=1;
        }
        else{
        this->currentRequests = numRequests;
        }
        std::cout<<"from uniform "<<this->mean<<"\n";
        return this->mean;
    }
}
