import os
import matplotlib.pyplot as plt
import numpy as np
# ---------------------------------------------------------------------------
# 📊 1. DATA INPUT SECTION 
# ---------------------------------------------------------------------------
tasks = ['Data_Loading', 'Basic_Stats', 'Histogram', 'Pearson_Corr', 'Outliers_Z', 'Moving_Avg', 'Sorting']
datasets = ['1M (Small)', '10M (Medium)', '100M (Large)']

# --- EXP-1, EXP-2 & EXP-3: Task Breakdown (Masa dalam Saat)
seq_task_times = {
    '1M (Small)':   [0, 0, 0, 0, 0, 0, 0], # BELUM BUAT: Sila run sequential untuk 1M dan isi sini
    '10M (Medium)': [18.9268, 0.0399, 0.0328, 0.0256, 0.0223, 0.0004, 0.9055], # SIAP (Dari log kau)
    '100M (Large)': [0, 0, 0, 0, 0, 0, 0]  # BELUM BUAT: Sila run sequential untuk 100M dan isi sini
}

mpi_task_times = {
    '1M (Small)':   [0, 0, 0, 0, 0, 0, 0], # BELUM BUAT: Sila run MPI 4-node untuk 1M
    '10M (Medium)': [12.4686, 0.0281, 0.0092, 0.0066, 0.0042, 0.0000002, 21.7856], # SIAP (Dari log MPI 4-nodes)
    '100M (Large)': [0, 0, 0, 0, 0, 0, 0]  # BELUM BUAT: Sila run MPI 4-node untuk 100M
}

# --- EXP-4: Communication vs Computation Breakdown (Data 10M 4-Node)
# Computation = Campuran masa Basic_Stats, Histogram, Pearson, Z_Score, Moving_Avg
# Overhead = Masa Loading + Masa Sorting (sebab Sorting MPI sangat lambat akibat hantar data)
pure_compute_time = 0.0481   # Hasil tambah tugasan mengira
mpi_network_overhead = 34.2542 # Hasil tambah Data_Loading + Sorting (MPI Overhead)

# --- EXP-5: Node Failure / Scaling Test (Masa Total_All_Tasks untuk data 10M Medium)
nodes_test = ['2 Nodes', '3 Nodes', '4 Nodes']
mpi_node_scaling_times = [26.4493, 0, 34.3025] # SIAP (Letak 0 dulu untuk 3 Nodes sebab belum run)

# --- Parameter Cluster & Amdahl's Law
N_NODES = 4
P_FRACTION = 0.92  # Peratusan kod yang boleh dipisahkan (92%)


# 2. MATRICES & MATHEMATICAL EVALUATIONS
os.makedirs('csc580_plots', exist_ok=True)
ts_totals = np.array([sum(seq_task_times[d]) for d in datasets])
tp_totals = np.array([sum(mpi_task_times[d]) for d in datasets])

# Formula Speedup: S = Ts / Tp
overall_speedup = ts_totals / tp_totals
# Formula Amdahl's Law Max Limit
theoretical_speedup = 1 / ((1 - P_FRACTION) + (P_FRACTION / N_NODES))

# Print Report Ringkas ke Terminal
print("="*60)
print("             CSC580 PERFORMANCE ANALYST REPORT              ")
print("="*60)
for i, d in enumerate(datasets):
    s_factor = overall_speedup[i]
    eff_pct = (s_factor / N_NODES) * 100
    overhead_val = (N_NODES * tp_totals[i]) - ts_totals[i]
    print(f"\n📈 DATASET SCALE: {d}")
    print(f"  • Speedup Factor (S)   : {s_factor:.2f}x")
    print(f"  • Parallel Efficiency (E): {eff_pct:.1f}%")
    print(f"  • Framework Overhead (O): {overhead_val:.4f} s")

# ---------------------------------------------------------------------------
# 📈 3. GENERATION OF THE 5 REQUIRED VISUALIZATIONS (Section 7.3)
# ---------------------------------------------------------------------------
plt.style.use('default')
bar_width = 0.35
x_indices = np.arange(len(tasks))

# --- PLOT 1: Execution Time Comparison per Analytical Task (100M) ---
plt.figure(figsize=(9, 5))
plt.bar(x_indices - bar_width/2, seq_task_times['10M (Medium)'], bar_width, label='Sequential (1 Node)', color='#e74c3c')
plt.bar(x_indices + bar_width/2, mpi_task_times['10M (Medium)'], bar_width, label='MPI Cluster (4 Nodes)', color='#2ecc71')
plt.xlabel('Analytical Task')
plt.ylabel('Execution Time (seconds, Log Scale)')
plt.title('Bar Chart — Execution Time Comparison per Analytical Task (100M Records)')
plt.xticks(x_indices, tasks, rotation=15)
plt.yscale('log')
plt.legend()
plt.tight_layout()
plt.savefig('csc580_plots/1_task_execution_time.png', dpi=300, bbox_inches='tight')
plt.close()

# --- PLOT 2: Speedup vs. Dataset Size (EXP-2) ---
plt.figure(figsize=(8, 5))
plt.plot(datasets, overall_speedup, marker='o', linewidth=2.5, color='#3498db', label='Observed Cluster Speedup')
plt.axhline(y=N_NODES, color='black', linestyle=':', label='Ideal Linear Limit (4.0x)')
plt.xlabel('Dataset Size Configuration')
plt.ylabel('Speedup Factor Score')
plt.title('Line Graph — Speedup vs. Dataset Size (EXP-2)')
plt.grid(True, linestyle=':', alpha=0.6)
plt.legend()
plt.tight_layout()
plt.savefig('csc580_plots/2_speedup_vs_size.png', dpi=300, bbox_inches='tight')
plt.close()

# --- PLOT 3: Grouped Bar Chart — Task-Level Time Breakdown (EXP-4 Insight) ---
plt.figure(figsize=(7, 5))
categories = ['Computation Volume', 'MPI Framework Overhead']
values = [pure_compute_time, mpi_network_overhead]
plt.bar(categories, values, color=['#34495e', '#e67e22'], width=0.4)
plt.ylabel('Time Spent (seconds)')
plt.title('Grouped Bar Chart — Task-Level Time Breakdown (Computation vs. Communication)')
for i, v in enumerate(values):
    plt.text(i, v + 2, f"{v}s", ha='center', fontweight='bold')
plt.tight_layout()
plt.savefig('csc580_plots/3_time_breakdown_overhead.png', dpi=300, bbox_inches='tight')
plt.close()

# --- PLOT 4: Efficiency Chart — Parallel Efficiency (%) per Task (100M) ---
plt.figure(figsize=(9, 5))
task_efficiency = [(seq_task_times['10M (Medium)'][i] / mpi_task_times['10M (Medium)'][i]) / N_NODES * 100 for i in range(len(tasks))]
bars = plt.bar(tasks, task_efficiency, color='#9b59b6', width=0.45, edgecolor='black')
plt.axhline(y=100, color='red', linestyle='--', label='Ideal Efficiency (100%)')
plt.xlabel('Analytical Task Elements')
plt.ylabel('Parallel Efficiency (%)')
plt.title('Efficiency Chart — Parallel Efficiency (%) per Task')

# FIXED: Dynamic vertical bound scaling to prevent outlier bars from piercing the chart limit
plt.ylim(0, max(task_efficiency) * 1.15) 
plt.xticks(rotation=15)

for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2.0, yval + 2, f'{yval:.1f}%', ha='center', va='bottom', fontsize=9)
plt.legend()
plt.tight_layout()
# FIXED: Added bbox_inches='tight' parameter to prevent label clipping inside file output
plt.savefig('csc580_plots/4_efficiency_per_task.png', dpi=300, bbox_inches='tight')
plt.close()

# --- PLOT 5: Amdahl\'s Law Overlay — Theoretical vs. Actual Speedup ---
plt.figure(figsize=(8, 5))
plt.plot(datasets, overall_speedup, marker='s', linewidth=2.5, color='#3498db', label='Actual MPI Speedup')
plt.axhline(y=theoretical_speedup, color='#e67e22', linestyle='--', linewidth=2, label=f"Amdahl's Theoretical Max (p={P_FRACTION*100:.0f}%)")
plt.axhline(y=N_NODES, color='black', linestyle=':', label='Ideal Target Limit (4.0x)')
plt.xlabel('Dataset Size')
plt.ylabel('Speedup Factor')
plt.title("Amdahl's Law Overlay — Theoretical vs. Actual Speedup")
plt.ylim(0, N_NODES + 0.5)
plt.grid(True, linestyle=':', alpha=0.6)
plt.legend(loc='lower right')
plt.tight_layout()
plt.savefig('csc580_plots/5_amdahls_law_overlay.png', dpi=300, bbox_inches='tight')
plt.close()

# --- EXTRA BONUS PLOT: EXP-5 Node Failure Optimization Analysis ---
plt.figure(figsize=(7, 5))
plt.bar(nodes_test, mpi_node_scaling_times, color='#1abc9c', width=0.4)
plt.xlabel('Active Cluster Configuration')
plt.ylabel('Execution Time (seconds)')
plt.title('EXP-5: Node Scaling Performance (Fault Tolerance Verification)')
plt.tight_layout()
plt.savefig('csc580_plots/bonus_exp5_node_scaling.png', dpi=300, bbox_inches='tight')
plt.close()

print("\n🚀 SIAP! Kesemua graf keperluan Seksyen 7.3 & Eksperimen 7.2 telah dieksport ke folder 'csc580_plots/'")