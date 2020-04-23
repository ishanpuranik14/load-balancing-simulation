#include "Poisson.h"
#include <bits/stdc++.h>
#include "spdlog/spdlog.h"

Poisson::Poisson(double lambda) : Generator() {
    this->lambda = lambda;
    spdlog::info("Using Poisson distribution with lambda: {}\n", lambda);
    std::random_device rd; // uniformly-distributed integer random number generator
    rng = std::mt19937(rd());
    exp = std::exponential_distribution<double>(lambda);
}

double Poisson::generate() {
    return exp.operator()(rng);
//    return -logf(1.0f - generate_random_number(0, 1)) / (this->lambda);
}
