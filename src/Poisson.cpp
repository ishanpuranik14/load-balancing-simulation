#include "Poisson.h"
#include<bits/stdc++.h>

Poisson::Poisson(double mean):Generator(){
            this->mean = mean;
        }

int Poisson::generate(){
            return -logf(1.0f - generate_random_number(0,1)) / (this->mean);
            // double L = exp(-(this->mean)), k=0, p=1;
            // do{
            //     k += 1;
            //     p *= generate_random_number(0,1);
            // } while (p>L);
            // return k-1;
        }
 