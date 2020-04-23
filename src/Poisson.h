#ifndef _POISSON_H
#define _POISSON_H
#include "Generator.h"

    class Poisson:public Generator{
        private:
            int lambda;
            int granularity;
        public:
            Poisson(int lambda,int granularity);
            double generate();
    };
#endif
 