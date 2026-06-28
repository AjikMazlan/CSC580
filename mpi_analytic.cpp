#include <mpi.h>

#include <iostream>
#include <vector>
#include <random>
#include <numeric>


using namespace std;



int main(int argc,char* argv[])
{


MPI_Init(&argc,&argv);



int rank,size;



MPI_Comm_rank(
MPI_COMM_WORLD,
&rank);



MPI_Comm_size(
MPI_COMM_WORLD,
&size);



long long N=10000000;



vector<double> data;



if(rank==0)
{

    data.resize(N);


    mt19937_64 rng(42);


    uniform_real_distribution<double>
    dist(0,10000);


    for(long long i=0;i<N;i++)
    {

        data[i]=dist(rng);

    }

}




MPI_Bcast(
&N,
1,
MPI_LONG_LONG,
0,
MPI_COMM_WORLD);



long long localSize=N/size;



vector<double> localData(localSize);



MPI_Scatter(
data.data(),
localSize,
MPI_DOUBLE,


localData.data(),
localSize,
MPI_DOUBLE,


0,
MPI_COMM_WORLD);



MPI_Barrier(
MPI_COMM_WORLD);



double start=MPI_Wtime();




// local calculation


double localSum=0;


for(double x:localData)
{

    localSum+=x;

}




double totalSum;



MPI_Reduce(

&localSum,

&totalSum,

1,

MPI_DOUBLE,

MPI_SUM,

0,

MPI_COMM_WORLD

);




double end=MPI_Wtime();



if(rank==0)
{

double mean=
totalSum/N;


cout<<"MPI Result\n";

cout<<"Mean: "
<<mean<<endl;


cout<<"Time: "
<<(end-start)*1000
<<" ms\n";

}




MPI_Finalize();


return 0;

}