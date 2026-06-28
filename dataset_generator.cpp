#include <iostream>
#include <fstream>
#include <random>

using namespace std;

int main(int argc, char* argv[])
{

    if(argc < 2)
    {
        cout << "Usage: dataset_generator size\n";
        return 1;
    }


    long long N = stoll(argv[1]);


    ofstream file("dataset.csv");


    mt19937_64 rng(42);

    uniform_real_distribution<double> dist(0.0,10000.0);


    for(long long i=0;i<N;i++)
    {
        file << dist(rng) << "\n";
    }


    file.close();


    cout<<"Dataset generated: "<<N<<" values\n";


    return 0;
}