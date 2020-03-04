#include "Poisson.h"
#include "Generator.h"
#include<bits/stdc++.h>
using namespace std;

int main(){
    Poisson p = Poisson(1.0/2.0);
    int num_iterations = 10000000;
    double sum=0;
    for(int i=0; i<num_iterations; i++){
        sum += p.generate();
    }
    cout<<"Average: "<<sum/num_iterations<<endl<<endl;
    return 0;
}