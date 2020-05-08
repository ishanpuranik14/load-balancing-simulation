#include "Poisson.h"
#include <bits/stdc++.h>
#include "spdlog/spdlog.h"

Poisson::Poisson(double lambda,int granularity) : Generator()
{
    this->lambda = lambda;
    this->granularity = granularity;
    //spdlog::info("Using Poisson distribution with lambda: {}\n", double(lambda*1.0/granularity*1.0));
}

double Poisson::generate()
{
    return -logf(1.0f - generate_random_number(0, 1)) / double(lambda/granularity*1.0);
    // double L = exp(-(this->mean)), k=0, p=1;
    // do{
    //     k += 1;
    //     p *= generate_random_number(0,1);
    // } while (p>L);
    // return k-1;
}
