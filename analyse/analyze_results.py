#!/usr/bin/env python3
"""
تحلیل نتایج شبیه‌سازی IoT Project
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path

# تنظیمات
# Scenario-specific labels for X-axis
scenario_labels = {
    'packet_loss': 'Packet Loss Rate (%)',
    'delay': 'Network Delay (ms)',
    'mobility': 'Mobility Speed (m/s)',
    'bg_traffic': 'Background Traffic (Mbps)',
    'conn_time': 'Connection Time (s)'
}



results_dir = Path("~/ns-3-dev/results/iot-project/raw").expanduser()
output_dir = Path("~/ns-3-dev/analysis/iot-project").expanduser()
output_dir.mkdir(parents=True, exist_ok=True)

protocols = ["TCP", "QUIC"]
scenarios = ["packet_loss", "delay", "mobility", "bg_traffic", "conn_time"]

# تنظیمات نمودار
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (10, 6)

def load_data():
    """بارگذاری تمام فایل‌های CSV"""
    data = {}
    for protocol in protocols:
        for scenario in scenarios:
            file_path = results_dir / f"{protocol}_{scenario}.csv"
            if file_path.exists():
                df = pd.read_csv(file_path)
                # میانگین‌گیری بر اساس param_value
                df_avg = df.groupby('param_value').mean(numeric_only=True).reset_index()
                # مرتب‌سازی بر اساس param_value
                df_avg = df_avg.sort_values('param_value').reset_index(drop=True)
                data[f"{protocol}_{scenario}"] = df_avg
                print(f"✓ بارگذاری شد: {file_path.name}")
            else:
                print(f"✗ فایل یافت نشد: {file_path}")
    return data

def plot_comparison(data, scenario, metric, ylabel, title):
    """رسم نمودار مقایسه‌ای"""
    fig, ax = plt.subplots()

    for protocol in protocols:
        key = f"{protocol}_{scenario}"
        if key in data:
            df = data[key]
            ax.plot(df['param_value'], df[metric], marker='o', label=protocol, linewidth=2, markersize=8)

    ax.set_xlabel(scenario_labels.get(scenario, 'Parameter Value'), fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_title(title, fontsize=14, fontweight='bold')
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3)

    filename = output_dir / f"{scenario}_{metric}.png"
    plt.tight_layout()
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"✓ ذخیره شد: {filename.name}")

def generate_plots(data):
    """تولید تمام نمودارها"""
    print("\n=== تولید نمودارها ===")

    # Packet Loss Scenario
    if any(f"{p}_packet_loss" in data for p in protocols):
        plot_comparison(data, "packet_loss", "avg_delay_ms", "Average Delay (ms)", "Impact of Packet Loss on Delay")
        plot_comparison(data, "packet_loss", "throughput_mbps", "Throughput (Mbps)", "Impact of Packet Loss on Throughput")

    # Delay Scenario
    if any(f"{p}_delay" in data for p in protocols):
        plot_comparison(data, "delay", "avg_delay_ms", "Average Delay (ms)", "Impact of Network Delay")
        plot_comparison(data, "delay", "throughput_mbps", "Throughput (Mbps)", "Throughput vs Network Delay")

    # Mobility Scenario
    if any(f"{p}_mobility" in data for p in protocols):
        plot_comparison(data, "mobility", "avg_delay_ms", "Average Delay (ms)", "Impact of Node Mobility on Delay")
        plot_comparison(data, "mobility", "loss_rate_pct", "Packet Loss (%)", "Impact of Node Mobility on Loss")

    # Background Traffic Scenario
    if any(f"{p}_bg_traffic" in data for p in protocols):
        plot_comparison(data, "bg_traffic", "avg_delay_ms", "Average Delay (ms)", "Impact of Background Traffic on Delay")
        plot_comparison(data, "bg_traffic", "throughput_mbps", "Throughput (Mbps)", "Throughput with Background Traffic")

    # Connection Time Scenario
    if any(f"{p}_conn_time" in data for p in protocols):
        plot_comparison(data, "conn_time", "conn_time_ms", "Connection Time (ms)", "Connection Establishment Time")

def generate_summary_table(data):
    """تولید جدول خلاصه"""
    print("\n=== تولید جدول خلاصه ===")

    summary = []
    for scenario in scenarios:
        for protocol in protocols:
            key = f"{protocol}_{scenario}"
            if key in data:
                df = data[key]
                summary.append({
                    'Protocol': protocol,
                    'Scenario': scenario,
                    'Avg Throughput (Mbps)': df['throughput_mbps'].mean(),
                    'Avg Delay (ms)': df['avg_delay_ms'].mean(),
                    'Avg Loss (%)': df['loss_rate_pct'].mean(),
                    'Avg Jitter (ms)': df['jitter_ms'].mean()
                })

    summary_df = pd.DataFrame(summary)
    summary_file = output_dir / "summary.csv"
    summary_df.to_csv(summary_file, index=False)
    print(f"✓ جدول خلاصه ذخیره شد: {summary_file.name}")

    # نمایش در ترمینال
    print("\n" + "="*80)
    print(summary_df.to_string(index=False))
    print("="*80)

def main():
    print("=== شروع تحلیل نتایج ===\n")

    # بارگذاری داده‌ها
    data = load_data()

    if not data:
        print("\n✗ هیچ داده‌ای یافت نشد!")
        return

    # تولید نمودارها
    generate_plots(data)

    # تولید جدول خلاصه
    generate_summary_table(data)

    print(f"\n✓ تحلیل کامل شد. نتایج در {output_dir} ذخیره شدند.")

if __name__ == "__main__":
    main()
