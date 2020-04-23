#ifndef _POISSON_H
#define _POISSON_H

#include <random>
#include "Generator.h"

class Poisson : public Generator {
private:
    double lambda;
    std::mt19937 rng;
    std::exponential_distribution<double> exp;

public:
    Poisson(double lambda);

    double generate();
};

#endif
 