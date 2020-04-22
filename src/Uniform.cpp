#include <bits/stdc++.h>
#include "Uniform.h"
#include "spdlog/spdlog.h"


Uniform::Uniform(double iat, int lambda = 1) : Generator() {
    this->iat = iat;
    this->lambda = lambda;
    this->counter = 0;
    spdlog::info("Using Uniform distribution with lambda: {} and IAT: {}\n", lambda, iat);
}

double Uniform::generate() {
    /*
    first, returns IAT, then for every IAT returns numRequests-1 0s
    */
    int current_iat = iat;
    if (counter) {
        current_iat = 0;
    }
    counter = (counter + 1) % lambda;
    spdlog::trace("Next request in : {} time units\n", current_iat);
    return current_iat;
}
