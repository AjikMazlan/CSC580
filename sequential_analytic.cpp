#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>

using namespace std;


vector<double> loadData()
{
    vector<double> data;
    // Removed the ./ prefix, using standard relative path
    ifstream file("Dataset/1Mdata.csv"); 

    // 1. Check if the file actually opened
    if (!file.is_open())
    {
        cout << "ERROR: Could not open the file! The path is wrong or the file is locked.\n";
        return data; 
    }

    cout << "SUCCESS: File opened successfully! Starting to read...\n";

    double x;
    while(file >> x)
    {
        data.push_back(x);
    }

    // 2. Check if the file was opened but no numbers were read
    if (data.empty())
    {
        cout << "ERROR: File was found, but no numbers were read.\n";
        cout << "Fix: Open 1Mdata.csv in Notepad. Is there a text header on the first row?\n";
    }

    file.close();
    return data;
}


//Basic Statistic
void statistics(vector<double>& data)
{

    double sum=0;


    double minValue=data[0];

    double maxValue=data[0];


    for(double x:data)
    {

        sum+=x;


        if(x<minValue)
            minValue=x;


        if(x>maxValue)
            maxValue=x;

    }


    double mean=sum/data.size();



    double variance=0;


    for(double x:data)
    {
        variance += pow(x-mean,2);
    }


    variance/=data.size();



    cout<<"Mean: "<<mean<<endl;

    cout<<"Variance: "<<variance<<endl;

    cout<<"Std: "<<sqrt(variance)<<endl;

    cout<<"Min: "<<minValue<<endl;

    cout<<"Max: "<<maxValue<<endl;

}



//Histogram Generation
void histogram(vector<double>& data)
{

    int bins[10]={0};


    for(double x:data)
    {

        int index=x/1000;


        if(index>=10)
            index=9;


        bins[index]++;

    }



    cout<<"\nHistogram\n";


    for(int i=0;i<10;i++)
    {
        cout<<"Bin "<<i<<" : "<<bins[i]<<endl;
    }

}


//moving average
void movingAverage(vector<double>& data)
{

    int window=5;


    double avg;


    cout<<"\nMoving Average sample:\n";


    for(int i=0;i<10;i++)
    {

        avg=0;


        for(int j=0;j<window;j++)
        {

            avg+=data[i+j];

        }


        cout<<avg/window<<endl;

    }

}



int main()
{


auto start =
chrono::high_resolution_clock::now();



vector<double> data=loadData();



cout<<"Data size: "
<<data.size()<<endl;



statistics(data);


histogram(data);



sort(data.begin(),data.end());


movingAverage(data);



auto end = chrono::high_resolution_clock::now();

// Remove 'milli' to default to seconds
double time = chrono::duration<double>(end - start).count();

// Update the output text to reflect the new unit
cout << "\nTotal time: " << time << " seconds\n";


return 0;

}