#include <mpi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct DataPoint { double f1, f2; };

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3) {
        if (rank == 0) cout << "Cara guna: mpiexec -n <cores> mpi_analytic.exe <nama_fail> <saiz_data>\n";
        MPI_Finalize();
        return 1;
    }

    string filename = argv[1];
    long long N = stoll(argv[2]);

    // ---------------- FIX: Kira sendcounts & displs (boleh handle baki tak genap) ----------------
    // Sebelum ni: if (N % size != 0) { error }  <-- DIBUANG, sebab tak perlu lagi
    vector<int> sendcounts(size), displs(size); // dalam UNIT DataPoint (bukan double)
    long long base_chunk = N / size;
    long long remainder  = N % size;
    long long offset = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (int)(base_chunk + (i < remainder ? 1 : 0));
        displs[i] = (int)offset;
        offset += sendcounts[i];
    }
    long long localSize = sendcounts[rank]; // saiz local untuk node ni sendiri

    vector<DataPoint> allData;
    vector<DataPoint> localData(localSize);
    double load_time_ms = 0;

    // ---------------- 0. DATA LOADING (MASTER SAHAJA) ----------------
    if (rank == 0) {
        cout << "--- Memuatkan " << N << " rekod dari fail: " << filename << " ---\n";
        auto start_load = high_resolution_clock::now();

        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Ralat: Fail '" << filename << "' tidak dijumpai di Master Node!\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        string line;
        getline(file, line); // Skip header
        for (int i = 0; i < N; ++i) {
            getline(file, line);
            stringstream ss(line);
            string v1, v2;
            if (getline(ss, v1, ',') && getline(ss, v2, ',')) {
                try {
                    allData.push_back({stod(v1), stod(v2)});
                } catch (...) {
                    allData.push_back({0.0, 0.0}); // Fallback jika data rosak
                }
            }
        }
        auto end_load = high_resolution_clock::now();
        load_time_ms = duration<double, milli>(end_load - start_load).count();
        cout << "    -> Masa Loading Data: " << load_time_ms << " ms\n\n";
    }

    // ---------------- FIX: SCATTERV gantikan SCATTER ----------------
    // Kira counts & displs dalam unit DOUBLE (setiap DataPoint = 2 double: f1, f2)
    vector<int> scatterCounts(size), scatterDispls(size);
    for (int i = 0; i < size; i++) {
        scatterCounts[i] = sendcounts[i] * 2;
        scatterDispls[i] = displs[i] * 2;
    }

    MPI_Scatterv(allData.empty() ? nullptr : allData.data(),
                 scatterCounts.data(), scatterDispls.data(), MPI_DOUBLE,
                 localData.data(), (int)(localSize * 2), MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    // Buka fail log di Master Node
    ofstream logFile;
    if (rank == 0) {
        logFile.open("mpi_timing_log.csv", ios::app);
        logFile << "\n--- RUN MPI: " << filename << " (" << N << " records, " << size << " nodes) ---\n";
        logFile << "Task,Time_ms,Time_Seconds\n";
        logFile << "Data_Loading," << load_time_ms << "," << (load_time_ms / 1000.0) << "\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double total_start = MPI_Wtime();

    // ---------------- TASK 1: BASIC STATS (Mean, StdDev, Min, Max) ----------------
    double t1_start = MPI_Wtime();
    double local_sum = 0, local_min = localData[0].f1, local_max = localData[0].f1;
    for (const auto& dp : localData) {
        local_sum += dp.f1;
        if (dp.f1 < local_min) local_min = dp.f1;
        if (dp.f1 > local_max) local_max = dp.f1;
    }

    double global_sum = 0, global_min = 0, global_max = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_min, &global_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max, &global_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    double global_mean = 0;
    if (rank == 0) global_mean = global_sum / N;
    MPI_Bcast(&global_mean, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double local_var_sum = 0;
    for (const auto& dp : localData) {
        local_var_sum += pow(dp.f1 - global_mean, 2);
    }

    double global_var_sum = 0;
    MPI_Reduce(&local_var_sum, &global_var_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    double global_stddev = 0;

    if (rank == 0) {
        global_stddev = sqrt(global_var_sum / N);
        double t1_end = MPI_Wtime();
        double elapsed = (t1_end - t1_start) * 1000.0;
        cout << "[Task 1] Mean: " << global_mean << " | StdDev: " << global_stddev
             << " | Min: " << global_min << " | Max: " << global_max << "\n";
        cout << "    -> Masa diambil: " << elapsed << " ms\n\n";
        logFile << "Basic_Stats," << elapsed << "," << elapsed / 1000.0 << "\n";
    }
    MPI_Bcast(&global_stddev, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); // FIX: semua node perlu stddev untuk Task 6

    // ---------------- TASK 2: HISTOGRAM ----------------
    double t2_start = MPI_Wtime();
    int local_bins[10] = {0};
    for (const auto& dp : localData) {
        int index = static_cast<int>(dp.f1 / 1000.0);
        if (index >= 10) index = 9;
        if (index < 0) index = 0;
        local_bins[index]++;
    }

    int global_bins[10] = {0};
    MPI_Reduce(local_bins, global_bins, 10, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double t2_end = MPI_Wtime();
        double elapsed = (t2_end - t2_start) * 1000.0;
        cout << "[Task 2] Histogram dikira (Bin 0: " << global_bins[0] << " ...)\n";
        cout << "    -> Masa diambil: " << elapsed << " ms\n\n";
        logFile << "Histogram," << elapsed << "," << elapsed / 1000.0 << "\n";
    }

    // ---------------- TASK 4: PEARSON CORRELATION ----------------
    double t4_start = MPI_Wtime();
    double local_sum_x = 0, local_sum_y = 0, local_sum_xy = 0, local_sum_x2 = 0, local_sum_y2 = 0;
    for (const auto& dp : localData) {
        local_sum_x += dp.f1;
        local_sum_y += dp.f2;
        local_sum_xy += (dp.f1 * dp.f2);
        local_sum_x2 += (dp.f1 * dp.f1);
        local_sum_y2 += (dp.f2 * dp.f2);
    }

    double g_sum_x, g_sum_y, g_sum_xy, g_sum_x2, g_sum_y2;
    MPI_Reduce(&local_sum_x, &g_sum_x, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum_y, &g_sum_y, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum_xy, &g_sum_xy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum_x2, &g_sum_x2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_sum_y2, &g_sum_y2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double numerator = (N * g_sum_xy) - (g_sum_x * g_sum_y);
        double denominator = sqrt(((N * g_sum_x2) - (g_sum_x * g_sum_x)) * ((N * g_sum_y2) - (g_sum_y * g_sum_y)));
        double correlation = (denominator == 0) ? 0 : numerator / denominator;

        double t4_end = MPI_Wtime();
        double elapsed = (t4_end - t4_start) * 1000.0;
        cout << "[Task 4] Pearson Correlation: " << correlation << "\n";
        cout << "    -> Masa diambil: " << elapsed << " ms\n\n";
        logFile << "Pearson_Correlation," << elapsed << "," << elapsed / 1000.0 << "\n";
    }

    // ---------------- TASK 6: Z-SCORE OUTLIERS ----------------
    double t6_start = MPI_Wtime();
    int local_outliers = 0;
    for (const auto& dp : localData) {
        double z_score = abs((dp.f1 - global_mean) / global_stddev);
        if (z_score > 3.0) local_outliers++;
    }

    int global_outliers = 0;
    MPI_Reduce(&local_outliers, &global_outliers, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double t6_end = MPI_Wtime();
        double elapsed = (t6_end - t6_start) * 1000.0;
        cout << "[Task 6] Z-Score Outliers dikesan: " << global_outliers << "\n";
        cout << "    -> Masa diambil: " << elapsed << " ms\n\n";
        logFile << "Z_Score_Outliers," << elapsed << "," << elapsed / 1000.0 << "\n";
    }

    // ---------------- TASK 5: MOVING AVERAGE ----------------
    double t5_start = MPI_Wtime();
    double first_avg = 0;
    if (rank == 0) {
        int window = 5;
        for(int j = 0; j < window; j++) first_avg += localData[j].f1;
        first_avg /= window;

        double t5_end = MPI_Wtime();
        double elapsed = (t5_end - t5_start) * 1000.0;
        cout << "[Task 5] Moving Average (Sample Pertama): " << first_avg << "\n";
        cout << "    -> Masa diambil: " << elapsed << " ms\n\n";
        logFile << "Moving_Average," << elapsed << "," << elapsed / 1000.0 << "\n";
    }

    // ---------------- TASK 3: SORTING ----------------
    double t3_start = MPI_Wtime();
    sort(localData.begin(), localData.end(), [](DataPoint a, DataPoint b) {
        return a.f1 < b.f1;
    });

    // FIX: GATHERV gantikan GATHER (localSize berbeza-beza ikut node sekarang)
    if (rank == 0 && (long long)allData.size() < N) allData.resize(N); // pastikan buffer cukup besar
    MPI_Gatherv(localData.data(), (int)(localSize * 2), MPI_DOUBLE,
                allData.empty() ? nullptr : allData.data(),
                scatterCounts.data(), scatterDispls.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        sort(allData.begin(), allData.end(), [](DataPoint a, DataPoint b) {
            return a.f1 < b.f1;
        });

        double t3_end = MPI_Wtime();
        double elapsed = (t3_end - t3_start) * 1000.0;
        cout << "[Task 3] Data berjaya disusun secara Parallel & Global.\n";
        cout << "    -> Masa diambil: " << elapsed << " ms\n\n";
        logFile << "Sorting," << elapsed << "," << elapsed / 1000.0 << "\n";
    }

    // ---------------- TAMAT & TUTUP LOG ----------------
    MPI_Barrier(MPI_COMM_WORLD);
    double total_end = MPI_Wtime();

    if (rank == 0) {
        double total_mpi_s = (total_end - total_start);
        double total_overall_s = total_mpi_s + (load_time_ms / 1000.0);

        cout << "=== TOTAL KESELURUHAN MASA (Termasuk Load): " << total_overall_s << " s ===\n";
        logFile << "Total_All_Tasks," << (total_overall_s * 1000.0) << "," << total_overall_s << "\n";
        logFile.close();
    }

    MPI_Finalize();
    return 0;
}