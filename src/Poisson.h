#ifndef _POISSON_H
#define _POISSON_H
#include "Generator.h"

    class Poisson:public Generator{
        private:
            double lambda;
            int granularity;
        public:
            Poisson(double lambda,int granularity);
            double generate();
    };
#endif
 