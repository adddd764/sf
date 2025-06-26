import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FuncFormatter

# 设置中文显示
plt.rcParams["font.family"] = ["SimHei", "WenQuanYi Micro Hei", "Heiti TC"]
plt.rcParams["axes.unicode_minus"] = False  # 正确显示负号


def read_performance_data(file_path):
    """读取并清洗算法性能数据"""
    try:
        # 读取CSV文件，将'-'转换为NaN
        df = pd.read_csv(file_path, na_values=['-'])

        # 确保所有时间列都是数值类型
        time_columns = [
            'Brute Force (ms)',
            'Dynamic Programming (ms)',
            'Greedy (Selection Sort) (ms)',
            'Greedy (Quick Sort) (ms)',
            'Backtracking (ms)'
        ]

        for col in time_columns:
            if col in df.columns:
                df[col] = pd.to_numeric(df[col], errors='coerce')

        return df
    except Exception as e:
        print(f"读取数据文件时出错: {e}")
        return None


def plot_small_scale_comparison(df, capacity):
    """绘制指定容量下小规模数据的算法性能对比图（回溯法 vs 蛮力法）"""
    if df is None:
        return

    # 筛选指定容量的数据
    capacity_df = df[df['Knapsack Capacity'] == capacity]

    # 筛选小规模数据（10-20个物品）
    small_counts = [10, 15, 20]
    small_df = capacity_df[capacity_df['Number of Items'].isin(small_counts)]

    # 如果没有数据则返回
    if small_df.empty:
        print(f"容量为{capacity}时没有小规模数据")
        return

    # 设置图表
    plt.figure(figsize=(10, 6))

    # 定义小规模算法
    small_algorithms = [
        {"name": "Brute Force (ms)", "label": "蛮力法", "color": "red"},
        {"name": "Backtracking (ms)", "label": "回溯法", "color": "blue"}
    ]

    # 绘制算法曲线
    for alg in small_algorithms:
        x = small_df['Number of Items']
        y = small_df[alg["name"]]
        plt.plot(x, y, marker='o', linestyle='-', linewidth=2,
                 color=alg["color"], label=alg["label"])

    # 设置图表属性
    plt.title(f'小规模数据下的算法性能对比 (容量={capacity})', fontsize=16)
    plt.xlabel('物品数量', fontsize=14)
    plt.ylabel('执行时间(ms)', fontsize=14)
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.legend(fontsize=12)
    plt.xticks(small_counts)  # 仅显示有数据的点

    # 优化图表样式
    plt.tight_layout()
    plt.gca().spines['top'].set_visible(False)
    plt.gca().spines['right'].set_visible(False)

    # 显示图表
    plt.show()


def plot_large_scale_comparison(df, capacity):
    """绘制指定容量下大规模数据的算法性能对比图（动态规划 vs 贪心算法）"""
    if df is None:
        return

    # 筛选指定容量的数据
    capacity_df = df[df['Knapsack Capacity'] == capacity]

    # 筛选大规模数据（1000+个物品）
    large_counts = sorted([c for c in capacity_df['Number of Items'].unique() if c >= 1000])
    large_df = capacity_df[capacity_df['Number of Items'].isin(large_counts)]

    # 如果没有数据则返回
    if large_df.empty:
        print(f"容量为{capacity}时没有大规模数据")
        return

    # 设置图表
    plt.figure(figsize=(12, 8))

    # 定义大规模算法
    large_algorithms = [
        {"name": "Dynamic Programming (ms)", "label": "动态规划", "color": "green"},
        {"name": "Greedy (Selection Sort) (ms)", "label": "贪心(选择排序)", "color": "orange"},
        {"name": "Greedy (Quick Sort) (ms)", "label": "贪心(快速排序)", "color": "purple"}
    ]

    # 绘制算法曲线
    for alg in large_algorithms:
        x = large_df['Number of Items']
        y = large_df[alg["name"]]
        plt.plot(x, y, marker='o', linestyle='-', linewidth=2,
                 color=alg["color"], label=alg["label"])

    # 设置对数坐标（如果数据范围大）
    if max(large_df['Number of Items']) / min(large_df['Number of Items']) > 100:
        plt.xscale('log')  # 使用对数坐标展示大范围数据

    # 设置图表属性
    plt.title(f'大规模数据下的算法性能对比 (容量={capacity})', fontsize=16)
    plt.xlabel('物品数量（对数刻度）', fontsize=14)
    plt.ylabel('执行时间(ms)', fontsize=14)
    plt.grid(axis='both', linestyle='--', alpha=0.7)
    plt.legend(fontsize=12)

    # 添加网格线和注释
    plt.xticks(large_counts, rotation=45)

    # 优化图表样式
    plt.tight_layout()
    plt.gca().spines['top'].set_visible(False)
    plt.gca().spines['right'].set_visible(False)

    # 显示图表
    plt.show()


def main():
    """主函数"""
    file_path = "knapsack_algorithm_performance.csv"
    df = read_performance_data(file_path)
    if df is None:
        print("无法读取数据，请检查文件路径")
        return

    # 获取数据中所有的容量值
    capacities = sorted(df['Knapsack Capacity'].unique())

    # 如果容量数量超过3个，选择最大的3个
    if len(capacities) > 3:
        capacities = capacities[-3:]

    print(f"将展示以下容量的对比图: {capacities}")

    # 为每个容量绘制两组对比图
    for capacity in capacities:
        plot_small_scale_comparison(df, capacity)
        plot_large_scale_comparison(df, capacity)

    print(f"已完成{len(capacities) * 2}张对比图的绘制")


if __name__ == "__main__":
    main()