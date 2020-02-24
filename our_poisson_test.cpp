//
// Created by ishan on 23/02/20.
//

#include <bits/stdc++.h>
using namespace std;

int poisson_generator(double mean){
    double L = exp(-mean), k=0, p=1;
    do
    {
        k += 1;
        int random_value= rand();
        // cout<<"Random value generated: "<<random_value<<endl;
        p *= ((float)random_value)/((float)RAND_MAX);
    } while (p>L);
    return k-1;
}

int main(){
    int num_iterations = 10000000;
    double sum=0;
    // Seeding the random generator
    srand (static_cast <unsigned> (time(0)));
    for(int i=0; i<num_iterations; i++){
        int generated_value = poisson_generator(5);
        // cout<<"Generated value: "<<generated_value<<endl;
        sum += generated_value;
    }
    cout<<"Average: "<<sum/num_iterations<<endl;
    return 0;
}
