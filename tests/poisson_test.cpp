//
// Created by ishan on 23/02/20.
//

#include <bits/stdc++.h>
using namespace std;

random_device rd;
mt19937 gen(rd());

int main(){
    poisson_distribution<int> pd(5);
    long double sum = 0;
    int num_iterations = 100000;
    for(int i=0; i< num_iterations; i++){
        sum += pd(gen);
    }
    cout<<"Average: "<<sum/num_iterations<<endl;
    return 0;
}
