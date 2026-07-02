import os
import matplotlib.pyplot as plt
import numpy as np
# ---------------------------------------------------------------------------
# 📊 1. DATA INPUT SECTION  (SEMUA DATA SIAP - dari log sequential & MPI)
# ---------------------------------------------------------------------------
tasks = ['Data_Loading', 'Basic_Stats', 'Histogram', 'Pearson_Corr', 'Outliers_Z', 'Moving_Avg', 'Sorting']
datasets = ['1M (Small)', '10M (Medium)', '100M (Large)']

# --- EXP-1, EXP-2 & EXP-3: Task Breakdown (Masa dalam Saat) ---
seq_task_times = {
    '1M (Small)':   [1.48024, 0.0044948, 0.0025182, 0.0018979, 0.0015175, 0.0003905, 0.0485854],
    '10M (Medium)': [15.411, 0.0300219, 0.0228553, 0.0173866, 0.0178549, 0.0003681, 0.665898],
    '100M (Large)': [747.951, 0.142667, 0.086487, 0.0639664, 0.0625385, 0.0005574, 4.38435],
}

# MPI task times guna konfigurasi 4-node (perbandingan utama sequential vs 4-node cluster)
mpi_task_times = {
    '1M (Small)':   [1.0032, 0.127646, 0.0122574, 0.0022329, 0.0013643, 2.00002e-07, 1.66211],
    '10M (Medium)': [11.8335, 0.019045, 0.0099496, 0.0079358, 0.0056148, 2.00002e-07, 15.9897],
    '100M (Large)': [131.669, 1.2401, 0.0975099, 0.0995717, 0.0479218, 2.00002e-07, 164.861],
}

# --- EXP-4: Communication vs Computation Breakdown (Data 10M, 4-Node) ---
# Computation = Basic_Stats + Histogram + Pearson + Z_Score + Moving_Avg
# Overhead = Data_Loading + Sorting (network-bound: scatter/gather)
pure_compute_time = 0.0425454      # 0.019045+0.0099496+0.0079358+0.0056148+0.0000002
mpi_network_overhead = 27.8232     # 11.8335 + 15.9897

# --- EXP-5: Node Scaling Test (Total_All_Tasks, data 10M Medium) ---
nodes_test = ['2 Nodes', '3 Nodes', '4 Nodes']
mpi_node_scaling_times = [20.655, 24.3403, 27.8661]

# --- Parameter Cluster & Amdahl's Law ---
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
    print(f"  • Sequential Total (Ts): {ts_totals[i]:.4f} s")
    print(f"  • MPI 4-Node Total (Tp): {tp_totals[i]:.4f} s")
    print(f"  • Speedup Factor (S)   : {s_factor:.2f}x")
    print(f"  • Parallel Efficiency (E): {eff_pct:.1f}%")
    print(f"  • Framework Overhead (O): {overhead_val:.4f} s")

# ---------------------------------------------------------------------------
# 📈 3. GENERATION OF THE 5 REQUIRED VISUALIZATIONS (Section 7.3)
# ---------------------------------------------------------------------------
plt.style.use('default')
bar_width = 0.35
x_indices = np.arange(len(tasks))

# --- PLOT 1: Execution Time Comparison per Analytical Task (10M) ---
# FIXED: Title now correctly matches the 10M dataset actually being plotted
plt.figure(figsize=(9, 5))
plt.bar(x_indices - bar_width/2, seq_task_times['10M (Medium)'], bar_width, label='Sequential (1 Node)', color='#e74c3c')
plt.bar(x_indices + bar_width/2, mpi_task_times['10M (Medium)'], bar_width, label='MPI Cluster (4 Nodes)', color='#2ecc71')
plt.xlabel('Analytical Task')
plt.ylabel('Execution Time (seconds, Log Scale)')
plt.title('Bar Chart — Execution Time Comparison per Analytical Task (10M Records)')
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
plt.axhline(y=1.0, color='gray', linestyle='--', linewidth=1, label='Break-even (1.0x)')
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
plt.title('Grouped Bar Chart — Task-Level Time Breakdown (Computation vs. Communication)\n[Data: 10M Records, 4 Nodes]')
for i, v in enumerate(values):
    plt.text(i, v + 0.5, f"{v:.4f}s", ha='center', fontweight='bold')
plt.ylim(0, max(values) * 1.2)
plt.tight_layout()
plt.savefig('csc580_plots/3_time_breakdown_overhead.png', dpi=300, bbox_inches='tight')
plt.close()

# --- PLOT 4: Efficiency Chart — Parallel Efficiency (%) per Task (10M) ---
# FIXED: Moving_Average excluded — its sequential time (0.0003681s) is too close
# to floating-point/timer resolution noise, producing a meaningless ~1800x ratio
# that would otherwise crush all other bars flat on the chart.
plot4_tasks = [t for t in tasks if t != 'Moving_Avg']
plot4_seq = [seq_task_times['10M (Medium)'][i] for i, t in enumerate(tasks) if t != 'Moving_Avg']
plot4_mpi = [mpi_task_times['10M (Medium)'][i] for i, t in enumerate(tasks) if t != 'Moving_Avg']
task_efficiency = [(plot4_seq[i] / plot4_mpi[i]) / N_NODES * 100 for i in range(len(plot4_tasks))]

plt.figure(figsize=(9, 5))
bars = plt.bar(plot4_tasks, task_efficiency, color='#9b59b6', width=0.45, edgecolor='black')
plt.axhline(y=100, color='red', linestyle='--', label='Ideal Efficiency (100%)')
plt.xlabel('Analytical Task Elements')
plt.ylabel('Parallel Efficiency (%)')
plt.title('Efficiency Chart — Parallel Efficiency (%) per Task\n(Moving_Avg excluded: near-zero baseline timing noise)')
plt.ylim(0, max(task_efficiency) * 1.15)
plt.xticks(rotation=15)

for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2.0, yval + 2, f'{yval:.1f}%', ha='center', va='bottom', fontsize=9)
plt.legend()
plt.tight_layout()
plt.savefig('csc580_plots/4_efficiency_per_task.png', dpi=300, bbox_inches='tight')
plt.close()

# --- PLOT 5: Amdahl's Law Overlay — Theoretical vs. Actual Speedup ---
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

# --- EXTRA BONUS PLOT: EXP-5 Node Scaling Analysis ---
plt.figure(figsize=(7, 5))
bars = plt.bar(nodes_test, mpi_node_scaling_times, color='#1abc9c', width=0.4, edgecolor='black')
plt.xlabel('Active Cluster Configuration')
plt.ylabel('Execution Time (seconds)')
plt.title('EXP-5: Node Scaling Performance (10M Records)')
for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2.0, yval + 0.3, f'{yval:.2f}s', ha='center', va='bottom', fontsize=9)
plt.tight_layout()
plt.savefig('csc580_plots/bonus_exp5_node_scaling.png', dpi=300, bbox_inches='tight')
plt.close()

print("\n🚀 SIAP! Kesemua graf keperluan Seksyen 7.3 & Eksperimen 7.2 telah dieksport ke folder 'csc580_plots/'")