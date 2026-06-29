#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <string>
#include <sstream>

using namespace std;
using namespace std::chrono;

// Struktur untuk menyimpan 2 column tanpa bercampur
struct DataPoint {
    double f1;
    double f2;
};

// Fungsi loadData kini merekodkan masa membaca
vector<DataPoint> loadData(string filename, int limit, double& load_time_ms) {
    vector<DataPoint> data;
    auto start = high_resolution_clock::now();
    
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Ralat: Gagal membuka fail " << filename << "\n";
        return data;
    }

    string line;
    getline(file, line); // Abaikan header

    int count = 0;
    while (getline(file, line) && count < limit) {
        stringstream ss(line);
        string val1, val2;
        if (getline(ss, val1, ',') && getline(ss, val2, ',')) {
            try {
                data.push_back({stod(val1), stod(val2)});
                count++;
            } catch (...) {}
        }
    }
    file.close();
    
    auto end = high_resolution_clock::now();
    load_time_ms = duration<double, milli>(end - start).count();
    return data;
}

// ---------------- TASK FUNCTIONS ----------------

void task1_statistics(const vector<DataPoint>& data, double& mean, double& stddev) {
    double sum = 0, min_val = data[0].f1, max_val = data[0].f1;
    for (const auto& dp : data) {
        sum += dp.f1;
        if (dp.f1 < min_val) min_val = dp.f1;
        if (dp.f1 > max_val) max_val = dp.f1;
    }
    
    mean = sum / data.size();
    double variance = 0;
    for (const auto& dp : data) {
        variance += pow(dp.f1 - mean, 2);
    }
    variance /= data.size();
    stddev = sqrt(variance);

    cout << "[Task 1] Mean: " << mean << " | StdDev: " << stddev 
         << " | Min: " << min_val << " | Max: " << max_val << endl;
}

void task2_histogram(const vector<DataPoint>& data) {
    int bins[10] = {0};
    for (const auto& dp : data) {
        int index = static_cast<int>(dp.f1 / 1000.0);
        if (index >= 10) index = 9;
        if (index < 0) index = 0;
        bins[index]++;
    }
    cout << "[Task 2] Histogram dikira (Bin 0: " << bins[0] << " ...)\n";
}

void task3_sorting(vector<DataPoint>& data) {
    // Susun berdasarkan feature_1 (f1)
    sort(data.begin(), data.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.f1 < b.f1;
    });
    cout << "[Task 3] Data berjaya disusun (Sorting).\n";
}

void task4_pearson(const vector<DataPoint>& data) {
    double sum_x = 0, sum_y = 0, sum_xy = 0;
    double sum_x2 = 0, sum_y2 = 0;
    int n = data.size();

    for (const auto& dp : data) {
        sum_x += dp.f1;
        sum_y += dp.f2;
        sum_xy += (dp.f1 * dp.f2);
        sum_x2 += (dp.f1 * dp.f1);
        sum_y2 += (dp.f2 * dp.f2);
    }

    double numerator = (n * sum_xy) - (sum_x * sum_y);
    double denominator = sqrt(((n * sum_x2) - (sum_x * sum_x)) * ((n * sum_y2) - (sum_y * sum_y)));
    
    double correlation = (denominator == 0) ? 0 : numerator / denominator;
    cout << "[Task 4] Pearson Correlation: " << correlation << endl;
}

void task5_movingAverage(const vector<DataPoint>& data) {
    int window = 5;
    if (data.size() < window) return;
    
    double first_avg = 0;
    for(int j = 0; j < window; j++) {
        first_avg += data[j].f1;
    }
    cout << "[Task 5] Moving Average (Sample Pertama): " << first_avg / window << endl;
}

void task6_outliers(const vector<DataPoint>& data, double mean, double stddev) {
    int outlier_count = 0;
    for (const auto& dp : data) {
        double z_score = abs((dp.f1 - mean) / stddev);
        if (z_score > 3.0) {
            outlier_count++;
        }
    }
    cout << "[Task 6] Z-Score Outliers dikesan: " << outlier_count << endl;
}

// ---------------- MAIN EXECUTOR ----------------

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Cara guna: sequential_analytic_2.exe <nama_fail> <saiz_data>\n";
        return 1;
    }
    
    string filename = argv[1];
    int dataSize = stoi(argv[2]);

    double load_time_ms = 0;
    cout << "--- Memuatkan " << dataSize << " rekod dari fail: " << filename << " ---\n";
    
    vector<DataPoint> data = loadData(filename, dataSize, load_time_ms);
    cout << "    -> Masa Loading Data: " << load_time_ms << " ms\n\n";
    
    if (data.empty()) return 1;

    ofstream logFile("sequential_timing_log.csv", ios::app);
    logFile << "\n--- RUN: " << filename << " (" << dataSize << " records) ---\n";
    logFile << "Task,Time_ms,Time_Seconds\n";
    
    // Rekod load time ke dalam log
    logFile << "Data_Loading," << load_time_ms << "," << (load_time_ms / 1000.0) << "\n";

    double mean = 0, stddev = 0;
    #define MEASURE_TIME(taskName, funcCall) { \
        auto start = high_resolution_clock::now(); \
        funcCall; \
        auto end = high_resolution_clock::now(); \
        double elapsed_s = duration<double>(end - start).count(); \
        double elapsed_ms = elapsed_s * 1000.0; \
        cout << "    -> Masa diambil: " << elapsed_ms << " ms (" << elapsed_s << " s)\n\n"; \
        logFile << taskName << "," << elapsed_ms << "," << elapsed_s << "\n"; \
    }

    auto total_start = high_resolution_clock::now();

    MEASURE_TIME("Basic_Stats", task1_statistics(data, mean, stddev));
    MEASURE_TIME("Histogram", task2_histogram(data));
    MEASURE_TIME("Pearson_Correlation", task4_pearson(data));
    MEASURE_TIME("Z_Score_Outliers", task6_outliers(data, mean, stddev));
    MEASURE_TIME("Moving_Average", task5_movingAverage(data));
    MEASURE_TIME("Sorting", task3_sorting(data));

    auto total_end = high_resolution_clock::now();
    double total_s = duration<double>(total_end - total_start).count() + (load_time_ms / 1000.0);
    
    cout << "=== TOTAL KESELURUHAN MASA: " << total_s << " s ===\n";
    logFile << "Total_All_Tasks," << (total_s * 1000.0) << "," << total_s << "\n";
    logFile.close();

    return 0;
}