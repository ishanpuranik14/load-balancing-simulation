#include <bits/stdc++.h>
#include "Uniform.h"
#include "spdlog/spdlog.h"

Uniform::Uniform(double mean) : Generator() {
    this->mean = mean;
    spdlog::info("Using Uniform distribution with lambda: {}\n", mean);
}

double Uniform::generate() {
    return this->mean;
}
