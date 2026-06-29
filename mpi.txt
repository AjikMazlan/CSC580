#include <mpi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

struct DataPoint { double f1, f2; };

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // [PENTING] Jadikan dinamik seperti kod sequential
    if (argc < 3) {
        if (rank == 0) cout << "Cara guna: mpiexec -n <cores> mpi_analytic.exe <nama_fail> <saiz_data>\n";
        MPI_Finalize();
        return 1;
    }

    string filename = argv[1];
    long long N = stoll(argv[2]);

    if (N % size != 0) {
        if (rank == 0) cout << "Ralat: Saiz data (" << N << ") tak boleh dibahagi tepat dengan jumlah node (" << size << ")!" << endl;
        MPI_Finalize();
        return 1;
    }

    long long localSize = N / size;
    vector<DataPoint> allData;
    vector<DataPoint> localData(localSize);

    // 1. MASTER NODE Membaca Fail CSV
    if (rank == 0) {
        cout << "Master: Membaca fail " << filename << " (" << N << " rekod)..." << endl;
        ifstream file(filename);
        
        if (!file.is_open()) {
            cout << "Ralat: Fail '" << filename << "' tidak dijumpai di Master Node!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        string line;
        getline(file, line); // Skip header
        for (int i = 0; i < N; ++i) {
            getline(file, line);
            stringstream ss(line);
            string v1, v2;
            getline(ss, v1, ','); getline(ss, v2, ',');
            allData.push_back({stod(v1), stod(v2)});
        }
    }

    // 2. SCATTER
    MPI_Scatter(allData.empty() ? nullptr : allData.data(), localSize * 2, MPI_DOUBLE,
                localData.data(), localSize * 2, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    // 3. ANALISIS PARALLEL (Asas)
    double localSum = 0;
    for (auto& p : localData) localSum += p.f1;

    sort(localData.begin(), localData.end(), [](DataPoint a, DataPoint b) {
        return a.f1 < b.f1;
    });

    // 4. REDUCE & GATHER
    double totalSum = 0;
    MPI_Reduce(&localSum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Gather(localData.data(), localSize * 2, MPI_DOUBLE,
               allData.empty() ? nullptr : allData.data(), localSize * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double end = MPI_Wtime();

    // 5. OUTPUT MASTER NODE
    if (rank == 0) {
        cout << "--- Analisis MPI Selesai ---" << endl;
        cout << "Mean: " << totalSum / N << endl;
        cout << "Masa Komputasi Parallel: " << (end - start) * 1000 << " ms" << endl;
    }

    MPI_Finalize();
    return 0;
}