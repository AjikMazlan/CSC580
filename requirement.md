# 📡 Group Assignment: Distributed Data Analytics Using Message Passing Interface (MPI)

**Course**: CSC580 Parallel Computing  (High Performance Computing / Distributed Computing)
**Programme**: CS230 / CDCS230 ( Semester 5)  
**Group Size**: 2 - 4 Members  
**Total Marks**: 100%  
**Submission Deadline**: *Week 14*  
**Mode**: Group Laboratory + Report + Presentation  

---

## 📌 1. Introduction and Motivation

Modern computing increasingly relies on distributed and parallel systems to handle large-scale data processing tasks. The **Message Passing Interface (MPI)** is the industry-standard communication protocol for programming parallel computers, enabling processes running on different machines to coordinate and exchange data over a network.

In this assignment, your group will build a **real distributed data analytics pipeline** that runs across **4 physical Windows laptops** (one per group member) connected over a local area network (LAN). You will implement the same analytical workload in both **plain sequential C++** and **MPI-based parallel C++**, then rigorously **measure, compare, and report performance metrics**.

This assignment simulates a realistic industry scenario where data engineers and HPC developers distribute computation across a cluster of nodes.

---

## 🎯 2. Learning Outcomes

Upon completing this assignment, students will be able to:

- [ ] Set up and configure a multi-node MPI cluster on Windows machines over a LAN
- [ ] Implement data partitioning and distribution strategies using MPI primitives
- [ ] Apply MPI collective operations (`MPI_Scatter`, `MPI_Gather`, `MPI_Reduce`, `MPI_Bcast`) to a real analytical workload
- [ ] Measure and interpret parallel performance metrics: speedup, efficiency, and scalability
- [ ] Critically compare sequential vs. parallel execution strategies
- [ ] Write a structured technical report and deliver a professional oral presentation

---

## 👥 3. Group Roles and Responsibilities

Each member must take ownership of a specific role. **All members must participate in running and testing the distributed system.** Roles define primary responsibility, not exclusivity.

| Role | Member | Primary Responsibility |
|------|--------|------------------------|
| **Node Master / Architect** | Member 1 | Network configuration, MPI cluster setup, master node logic, job scheduling |
| **Algorithm Developer** | Member 2 | Sequential C++ implementation, MPI parallel algorithm design and coding |
| **Data Engineer** | Member 3 | Dataset preparation, data partitioning strategy, data validation |
| **Performance Analyst** | Member 4 | Benchmarking framework, metrics collection, performance graphs and analysis |

> ⚠️ **Important**: Every member must be present during the live distributed execution demonstration. Each laptop must function as an active MPI node.

---

## 🖥️ 4. Hardware and Network Setup Requirements

### 4.1 Hardware Requirements
Each group member must provide:
- A **Windows 10 / Windows 11** laptop
- Minimum **4 GB RAM**, recommended 8 GB or above
- Minimum **dual-core processor**
- A working **Wi-Fi or Ethernet adapter**

### 4.2 Network Configuration

All 4 laptops must be connected to the **same Local Area Network (LAN)** — either:
- Connected to the same Wi-Fi router/access point, **or**
- Connected via a network switch using Ethernet cables (recommended for better latency)

```
[Member 1 - Master Node]  ── LAN ──  [Member 2 - Worker Node]
         │                                      │
        LAN                                    LAN
         │                                      │
[Member 3 - Worker Node]  ── LAN ──  [Member 4 - Worker Node]
```

**Master Node (Member 1's laptop)** is responsible for:
- Launching the MPI job using `mpiexec`
- Distributing tasks to all 3 worker nodes
- Collecting results from all nodes

### 4.3 MPI Software Installation (Windows)

Install **Microsoft MPI (MS-MPI)** on all 4 machines:

**Step-by-step setup:**

```
Step 1: Download MS-MPI from:
        https://learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi

Step 2: Install both:
        - msmpisdk.msi (SDK for compilation)
        - msmpisetup.exe (runtime for all nodes)

Step 3: Set Environment Variables (System Path):
        C:\Program Files\Microsoft MPI\Bin\

Step 4: Configure Windows Firewall:
        Allow "Microsoft MPI Launch Service" through firewall
        Open TCP/UDP port range: 49152–65535

Step 5: Enable "smpd" service on all machines:
        Run as Administrator:
        > smpd -install
        > net start smpd

Step 6: Verify connectivity from Master node:
        > mpiexec -hosts 4 <IP1> 1 <IP2> 1 <IP3> 1 <IP4> 1 hostname
```

### 4.4 Hostfile Configuration (on Master Node)

Create a file named `hostfile.txt` on Member 1's laptop:

```
192.168.x.x1   # Member 1 (Master)
192.168.x.x2   # Member 2 (Worker)
192.168.x.x3   # Member 3 (Worker)
192.168.x.x4   # Member 4 (Worker)
```

> 📝 Replace IP addresses with the actual IPs of each laptop on the LAN. Confirm with `ipconfig` on each machine.

---

## 📊 5. Project: Distributed Statistical Data Analytics Pipeline

### 5.1 Problem Statement

Your group will implement a **distributed data analytics pipeline** that processes a **large numerical dataset** (minimum 10 million data points). The pipeline performs the following analytical computations:

| # | Analytical Task | Description |
|---|----------------|-------------|
| 1 | **Basic Statistics** | Compute mean, variance, standard deviation, min, max |
| 2 | **Histogram Generation** | Build a frequency histogram with configurable bin count |
| 3 | **Sorting** | Distributed parallel sort (e.g., Bitonic Sort / Sample Sort) |
| 4 | **Pearson Correlation** | Compute correlation between two data columns |
| 5 | **Moving Average** | Compute rolling window moving average |
| 6 | **Outlier Detection** | Identify outliers using Z-score method (|Z| > 3) |

### 5.2 Dataset

Use the following approaches to obtain a sufficiently large dataset:

**Option A (Recommended) — Generate Synthetic Data:**
```cpp
// Generate N random doubles in range [0, 10000]
// Use a fixed seed for reproducibility across runs
#include <random>
std::mt19937_64 rng(42);
std::uniform_real_distribution<double> dist(0.0, 10000.0);
```

**Option B — Use a Real-World Dataset:**  
- Download from [Kaggle](https://www.kaggle.com) or [UCI ML Repository](https://archive.ics.uci.edu)  
- Must have at least **10 million numerical records**  
- Pre-process into a clean binary or CSV format

> 📝 **Data Scale Requirements:**

| Run Size | Data Points | Expected Sequential Time |
|----------|------------|--------------------------|
| Small    | 1,000,000  | < 5 seconds              |
| Medium   | 10,000,000 | 10–60 seconds            |
| Large    | 100,000,000| 1–10 minutes             |

The group must run all three sizes for benchmarking.

---

## 💻 6. Implementation Requirements

### 6.1 Sequential C++ Implementation (`sequential_analytics.cpp`)

Implement all 6 analytical tasks in a **single-threaded, sequential** C++ program. This serves as the **baseline** for performance comparison.

**Minimum required features:**
- Command-line argument to specify dataset size: `./sequential_analytics 10000000`
- Wall-clock timing using `<chrono>` for each analytical task and total runtime
- Output results to both console and a CSV log file

```cpp
// Required timing structure
#include <chrono>
auto start = std::chrono::high_resolution_clock::now();
// ... computation ...
auto end = std::chrono::high_resolution_clock::now();
double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
```

### 6.2 MPI Parallel Implementation (`mpi_analytics.cpp`)

Implement all 6 analytical tasks using **MS-MPI** across 4 physical nodes.

**Required MPI operations (at minimum):**

| MPI Function | Required Use Case |
|-------------|------------------|
| `MPI_Init` / `MPI_Finalize` | Program lifecycle |
| `MPI_Comm_rank` / `MPI_Comm_size` | Node identification |
| `MPI_Bcast` | Broadcast dataset size / parameters to all nodes |
| `MPI_Scatter` / `MPI_Scatterv` | Distribute data chunks to worker nodes |
| `MPI_Gather` / `MPI_Gatherv` | Collect partial results from workers |
| `MPI_Reduce` | Aggregate statistical results (sum, min, max) |
| `MPI_Barrier` | Synchronize nodes for timing accuracy |
| `MPI_Wtime` | Precise timing across MPI runtime |

**Compilation command (on all nodes):**
```bat
cl /EHsc /O2 mpi_analytics.cpp /I "%MSMPI_INC%" /link "%MSMPI_LIB64%\msmpi.lib" /out:mpi_analytics.exe
```

**Execution command (from Master Node):**
```bat
mpiexec -n 4 -hosts 4 192.168.x.1 1 192.168.x.2 1 192.168.x.3 1 192.168.x.4 1 mpi_analytics.exe 10000000
```

### 6.3 Suggested MPI Data Distribution Strategy

```
Master Node (Rank 0):
  1. Generate or load full dataset
  2. MPI_Bcast: broadcast N (data size) to all nodes
  3. MPI_Scatterv: send N/4 elements to each node
  
Worker Nodes (Rank 1, 2, 3):
  1. Receive local data chunk
  2. Compute local statistics on chunk
  3. MPI_Reduce: send local results to master
  
Master Node:
  1. Combine results from all workers
  2. Output final analytics
  3. Record total wall-clock time
```

---

## 📈 7. Performance Analysis Requirements

This is the **core deliverable** of the assignment. Your group must conduct a rigorous, structured performance comparison.

### 7.1 Metrics to Measure and Report

| Metric | Formula | Description |
|--------|---------|-------------|
| **Execution Time (T)** | Measured (ms/s) | Wall-clock time for each task |
| **Speedup (S)** | $S = T_{seq} / T_{par}$ | How many times faster MPI is |
| **Efficiency (E)** | $E = S / P \times 100\%$ | Fraction of ideal speedup achieved (P = 4 nodes) |
| **Parallel Overhead** | $T_{overhead} = P \cdot T_{par} - T_{seq}$ | Cost of communication and synchronization |
| **Amdahl's Law Estimate** | $S_{max} = 1 / (f + (1-f)/P)$ | Theoretical maximum speedup |

### 7.2 Required Experiments

| Experiment | What to Test | Purpose |
|-----------|-------------|---------|
| **EXP-1: Baseline Comparison** | Sequential vs. MPI (4 nodes), medium dataset | Core comparison |
| **EXP-2: Scalability Test** | Small / Medium / Large dataset on MPI | Weak and strong scaling |
| **EXP-3: Task-Level Analysis** | Time breakdown per analytical task | Identify bottlenecks |
| **EXP-4: Communication Overhead** | Time spent in MPI calls vs. computation | Network impact |
| **EXP-5: Node Failure Simulation** | Run with 2 nodes vs. 3 nodes vs. 4 nodes | Fault tolerance discussion |

### 7.3 Required Output Visualizations

Your report and presentation **must include** the following plots:

1. **Bar Chart** — Execution time comparison (Sequential vs. MPI) per analytical task
2. **Line Graph** — Speedup vs. Dataset Size (EXP-2)
3. **Grouped Bar Chart** — Task-level time breakdown for MPI across 4 nodes
4. **Efficiency Chart** — Parallel efficiency (%) per task
5. **Amdahl's Law Overlay** — Theoretical vs. actual speedup

> 💡 **Tools Recommended**: Python (`matplotlib`, `pandas`), Excel, or MATLAB for plotting.

---

## 📝 8. Deliverables

| # | Deliverable | Format | Deadline |
|---|------------|--------|----------|
| D1 | **Source Code** | `.cpp` files + build script, hosted on GitHub | Week X |
| D2 | **Technical Report** | PDF, min. 15 pages | Week X+1 |
| D3 | **Live Demonstration** | In-lab with all 4 laptops running | Week X+1 |
| D4 | **Oral Presentation** | 15–20 minutes + Q&A | Week X+2 |
| D5 | **Peer Evaluation Form** | Individual submission | Week X+2 |

### 8.1 Technical Report Structure

1. **Title Page** — Group members, roles, date
2. **Abstract** — Summary of work and key findings (250 words)
3. **Introduction** — Background, motivation, problem statement
4. **System Setup** — Hardware specs, network config, MPI installation steps
5. **Algorithm Design** — Sequential and MPI algorithm descriptions, pseudocode, flowcharts
6. **Implementation** — Key code snippets with explanations for each analytical task
7. **Experimental Setup** — Dataset description, experiment configurations
8. **Results and Discussion** — All performance charts, tables, and analysis
9. **Challenges and Lessons Learned** — Technical issues encountered and solutions
10. **Conclusion** — Summary and future work
11. **Individual Contributions Table** — What each member did
12. **References** — IEEE format

---

## 🏆 9. Assessment Rubrics (100 Marks)

---

### Section A: Network Configuration and MPI Cluster Setup (15 Marks)

| Criterion | Excellent (4–5) | Good (3) | Satisfactory (2) | Needs Improvement (0–1) |
|-----------|----------------|----------|-----------------|------------------------|
| **A1. Network Setup** (5 marks) | All 4 laptops connected and verified; static IPs or hostfile correct; documented with screenshots | 3–4 laptops connected successfully with minor issues | 2 nodes working; partial network setup | Less than 2 nodes; no evidence of LAN configuration |
| **A2. MPI Installation & Configuration** (5 marks) | MS-MPI correctly installed on all 4 nodes; smpd configured; firewall rules applied; verified with `hostname` test | Installed on 3+ nodes with functional cluster | Installed on 2 nodes; partial functionality | Installed but non-functional; or missing from most nodes |
| **A3. Hostfile and mpiexec Setup** (5 marks) | Hostfile correctly configured; `mpiexec` launches jobs across all 4 nodes seamlessly | Launches on 3 nodes; minor configuration error | Launches on 2 nodes; significant config issues | Cannot launch distributed job |

---

### Section B: Implementation Quality (30 Marks)

| Criterion | Excellent (13–15) | Good (9–12) | Satisfactory (5–8) | Needs Improvement (0–4) |
|-----------|-------------------|-------------|---------------------|------------------------|
| **B1. Sequential C++ Implementation** (15 marks) | All 6 analytical tasks correctly implemented; clean, modular code; accurate results; proper timing with `<chrono>` | 5 tasks correctly implemented; mostly clean code; timing present | 3–4 tasks implemented; some bugs; timing partially implemented | Fewer than 3 tasks; significant bugs; no timing |
| **B2. MPI Parallel Implementation** (15 marks) | All 6 tasks parallelized using appropriate MPI primitives; correct data distribution and result aggregation; MPI timing used | 5 tasks parallelized correctly; minor issues in aggregation | 3–4 tasks parallelized; some incorrect MPI usage | Fewer than 3 tasks; major MPI errors; does not run on 4 nodes |

---

### Section C: Live Demonstration (20 Marks)

| Criterion | Excellent (18–20) | Good (13–17) | Satisfactory (8–12) | Needs Improvement (0–7) |
|-----------|-------------------|--------------|---------------------|------------------------|
| **C1. Distributed Execution** (10 marks) | MPI job runs successfully across all 4 physical laptops; all nodes active; output verified correct | Runs on 3 nodes; 1 node partially participating | Runs on 2 nodes; significant fallback to local execution | Runs only on 1 machine (no true distributed execution) |
| **C2. Live Performance Comparison** (5 marks) | Sequential and MPI runs demonstrated live with clear timing results; speedup visible and explained | Demonstrated with minor issues; results mostly clear | Only MPI or only sequential demonstrated live | Cannot demonstrate live comparison |
| **C3. All Members Active Participation** (5 marks) | All 4 members clearly demonstrate individual roles during demo; able to answer questions about their part | 3 members actively engaged | 2 members engaged; others passive | Only 1 member presents |

---

### Section D: Performance Analysis (20 Marks)

| Criterion | Excellent (18–20) | Good (13–17) | Satisfactory (8–12) | Needs Improvement (0–7) |
|-----------|-------------------|--------------|---------------------|------------------------|
| **D1. Completeness of Experiments** (10 marks) | All 5 experiments (EXP 1–5) conducted; all 3 dataset sizes tested; results tabulated clearly | 4 experiments with 2–3 dataset sizes | 3 experiments with 1–2 dataset sizes | 1–2 experiments; incomplete results |
| **D2. Visualizations and Interpretation** (10 marks) | All 5 required charts produced; axes labeled; trends correctly identified; insightful discussion linking results to theory (Amdahl's Law, overhead) | 4 charts; mostly correct interpretation | 2–3 charts; superficial analysis | 1 chart or missing; no meaningful analysis |

---

### Section E: Technical Report (10 Marks)

| Criterion | Excellent (9–10) | Good (7–8) | Satisfactory (5–6) | Needs Improvement (0–4) |
|-----------|-----------------|------------|---------------------|------------------------|
| **E1. Structure and Completeness** (5 marks) | All 12 report sections present; well-organized; meets 15-page minimum; professional formatting | 10–11 sections; mostly organized; near 15 pages | 7–9 sections; some disorganization | Fewer than 7 sections; poorly structured |
| **E2. Technical Depth and Clarity** (5 marks) | Pseudocode/flowcharts included; algorithm explanations are accurate and clear; code snippets well-commented and contextualized | Good explanations with minor gaps; most code explained | Basic explanations; some inaccuracies; limited code commentary | Explanations missing or incorrect throughout |

---

### Section F: Oral Presentation (5 Marks)

| Criterion | Excellent (5) | Good (4) | Satisfactory (3) | Needs Improvement (0–2) |
|-----------|--------------|----------|-----------------|------------------------|
| **F1. Delivery and Time Management** | All members present clearly; 15–20 min duration; professional slides; confident Q&A responses | 3 members present; within time; minor Q&A gaps | 2 members present; slightly over/under time | 1 member presents; poorly structured |

---

### Summary Score Table

| Section | Description | Max Marks |
|---------|-------------|-----------|
| A | Network Configuration and MPI Cluster Setup | 15 |
| B | Implementation Quality | 30 |
| C | Live Demonstration | 20 |
| D | Performance Analysis | 20 |
| E | Technical Report | 10 |
| F | Oral Presentation | 5 |
| **Total** | | **100** |

---

## 📐 10. Marking Adjustments

### 10.1 Peer Evaluation Weighting
Each group member submits a confidential **peer evaluation form** scoring the contribution of their teammates (0–10 scale). A multiplier is applied:

| Average Peer Score | Multiplier Applied to Individual Mark |
|--------------------|-----------------------------------------|
| 9.0 – 10.0 | × 1.00 (full mark retained) |
| 7.0 – 8.9  | × 0.95 |
| 5.0 – 6.9  | × 0.85 |
| < 5.0       | × 0.70 |

### 10.2 Academic Integrity
- All code must be **original**. Code similarity tools will be applied.
- Use of **AI-generated code** without disclosure is a violation.
- Each member must be able to **explain any part of the submitted code** during Q&A.

---

## 🔑 11. Helpful Resources

| Resource | Link |
|----------|------|
| MS-MPI Documentation | https://learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi |
| MPI Tutorial (Beginner) | https://mpitutorial.com |
| Open MPI Collective Ops | https://www.open-mpi.org/doc/v4.1/ |
| Amdahl's Law Explained | https://en.wikipedia.org/wiki/Amdahl%27s_law |
| C++ `<chrono>` Timing | https://en.cppreference.com/w/cpp/chrono |
| GitHub (Code Hosting) | https://github.com |

---

## 📋 12. Quick Checklist Before Submission

- [ ] All 4 laptops successfully configured as MPI nodes
- [ ] Sequential C++ program implements all 6 analytical tasks with timing
- [ ] MPI program implements all 6 analytical tasks with MPI timing
- [ ] All 5 experiments conducted and documented
- [ ] All 5 required performance charts included in report
- [ ] Amdahl's Law analysis included
- [ ] Source code pushed to GitHub repository
- [ ] Report is at least 15 pages, in PDF format
- [ ] All members present for live demo
- [ ] Peer evaluation forms submitted individually

---

*Document prepared by: Course Lecturer*  
*Last updated: June 2026*
