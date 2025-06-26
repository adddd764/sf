#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <limits.h>

// 物品结构体
typedef struct {
    int id;         // 物品编号
    int weight;     // 物品重量
    double value;   // 物品价值
    double ratio;   // 价值重量比
} Item;

// 全局变量用于记录内存占用峰值和算法执行时间
size_t max_memory_used = 0;
// 定义算法执行时间存储结构
typedef struct {
    double brute_force;
    double dynamic_programming;
    double greedy_selection;  // 原选择排序贪心算法
    double greedy_quick;      // 快速排序贪心算法
    double backtracking;
} AlgorithmTime;

// 定义测试参数与结果存储结构
typedef struct {
    int item_count;
    int capacity;
    AlgorithmTime time;
} TestResult;

// 内存分配跟踪函数
void* track_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr) {
        size_t current_memory = max_memory_used + size;
        if (current_memory > max_memory_used) {
            max_memory_used = current_memory;
        }
    }
    return ptr;
}

void track_free(void *ptr, size_t size) {
    free(ptr);
    max_memory_used -= size;
}

// 生成随机物品数组
Item* generate_items(int n) {
    Item *items = (Item*)track_malloc(n * sizeof(Item));
    if (!items) return NULL;
    
    for (int i = 0; i < n; i++) {
        items[i].id = i + 1;
        items[i].weight = rand() % 100 + 1;  // 1~100的随机重量
        items[i].value = (rand() % 9001) / 100.0 + 100; // 100.00~1000.00的随机价值
        items[i].ratio = items[i].value / items[i].weight;
    }
    return items;
}

// 输出物品信息到CSV文件
void export_to_csv(Item *items, int n, int capacity) {
    char filename[50];
    sprintf(filename, "背包容量%d_物品数量%d.csv", capacity, n);
    
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("无法创建CSV文件\n");
        return;
    }
    
    // 写入表头
    fprintf(fp, "物品编号,物品重量,物品价值\n");
    
    // 写入物品数据
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%.2f,%.2f\n", items[i].id, 
                (double)items[i].weight, items[i].value);
    }
    
    fclose(fp);
    printf("物品信息已导出到: %s\n\n", filename);
}

// 蛮力法求解
double brute_force_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(Item) * n; // 重置内存统计
    
    double max_value = 0;
    int best_selection = 0;
    long total = (long)1 << n; // 2^n种可能性
    
    // 遍历所有子集
    for (long i = 0; i < total; i++) {
        int current_weight = 0;
        double current_value = 0;
        
        for (int j = 0; j < n; j++) {
            if (i & (1L << j)) {
                current_weight += items[j].weight;
                current_value += items[j].value;
            }
        }
        
        if (current_weight <= capacity && current_value > max_value) {
            max_value = current_value;
            best_selection = i;
        }
    }
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("Brute Force Algorithm (Items=%d, Capacity=%d):\n", n, capacity);
    printf("Maximum Value: %.2f\n", max_value);
    printf("Selected Items: ");
    for (int j = 0; j < n; j++) {
        if (best_selection & (1L << j)) {
            printf("%d ", items[j].id);
        }
    }
    printf("\nExecution Time: %.2f ms\n", time_taken);
    printf("Memory Usage: %zu bytes\n\n", max_memory_used);
    
    return time_taken;
}

// 动态规划法求解
double dp_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(double) * (capacity + 1) + sizeof(Item) * n;
    
    // 创建DP表
    double *dp = (double*)track_malloc((capacity + 1) * sizeof(double));
    if (!dp) return -1;
    
    // 初始化DP表
    for (int w = 0; w <= capacity; w++) {
        dp[w] = 0;
    }
    
    // 动态规划求解
    for (int i = 0; i < n; i++) {
        for (int w = capacity; w >= items[i].weight; w--) {
            if (dp[w] < dp[w - items[i].weight] + items[i].value) {
                dp[w] = dp[w - items[i].weight] + items[i].value;
            }
        }
    }
    
    double max_value = dp[capacity];
    clock_t end = clock();
    double time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("Dynamic Programming Algorithm (Items=%d, Capacity=%d):\n", n, capacity);
    printf("Maximum Value: %.2f\n", max_value);
    printf("Execution Time: %.2f ms\n", time_taken);
    printf("Memory Usage: %zu bytes\n\n", max_memory_used);
    
    track_free(dp, (capacity + 1) * sizeof(double));
    return time_taken;
}

// 快速排序的比较函数
int compare_items(const void *a, const void *b) {
    Item *item1 = (Item *)a;
    Item *item2 = (Item *)b;
    if (item1->ratio < item2->ratio) return 1;   // 降序排列
    if (item1->ratio > item2->ratio) return -1;
    return 0;
}

// 选择排序的贪心算法（原版本）
double greedy_selection_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(Item) * n + sizeof(int) * n;
    
    // 按价值重量比排序（选择排序）
    Item *sorted_items = (Item*)track_malloc(n * sizeof(Item));
    if (!sorted_items) return -1;
    memcpy(sorted_items, items, n * sizeof(Item));
    
    // 简单选择排序
    for (int i = 0; i < n - 1; i++) {
        int max_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (sorted_items[j].ratio > sorted_items[max_idx].ratio) {
                max_idx = j;
            }
        }
        if (max_idx != i) {
            Item temp = sorted_items[i];
            sorted_items[i] = sorted_items[max_idx];
            sorted_items[max_idx] = temp;
        }
    }
    
    // 贪心选择
    int current_weight = 0;
    double total_value = 0;
    int *selected = (int*)track_malloc(n * sizeof(int));
    if (!selected) return -1;
    memset(selected, 0, n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        if (current_weight + sorted_items[i].weight <= capacity) {
            selected[i] = 1;
            current_weight += sorted_items[i].weight;
            total_value += sorted_items[i].value;
        }
    }
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("Greedy (Selection Sort) Algorithm (Items=%d, Capacity=%d):\n", n, capacity);
    printf("Maximum Value: %.2f\n", total_value);
    printf("Selected Items: ");
    for (int i = 0; i < n; i++) {
        if (selected[i]) {
            printf("%d ", sorted_items[i].id);
        }
    }
    printf("\nExecution Time: %.2f ms\n", time_taken);
    printf("Memory Usage: %zu bytes\n\n", max_memory_used);
    
    track_free(sorted_items, n * sizeof(Item));
    track_free(selected, n * sizeof(int));
    return time_taken;
}

// 快速排序的贪心算法（新版本）
double greedy_quick_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(Item) * n + sizeof(int) * n;
    
    // 按价值重量比排序（快速排序）
    Item *sorted_items = (Item*)track_malloc(n * sizeof(Item));
    if (!sorted_items) return -1;
    memcpy(sorted_items, items, n * sizeof(Item));
    
    // 使用qsort进行快速排序
    qsort(sorted_items, n, sizeof(Item), compare_items);
    
    // 贪心选择
    int current_weight = 0;
    double total_value = 0;
    int *selected = (int*)track_malloc(n * sizeof(int));
    if (!selected) return -1;
    memset(selected, 0, n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        if (current_weight + sorted_items[i].weight <= capacity) {
            selected[i] = 1;
            current_weight += sorted_items[i].weight;
            total_value += sorted_items[i].value;
        }
    }
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果(英文)
    printf("Greedy (Quick Sort) Algorithm (Items=%d, Capacity=%d):\n", n, capacity);
    printf("Maximum Value: %.2f\n", total_value);
    printf("Selected Items: ");
    for (int i = 0; i < n; i++) {
        if (selected[i]) {
            printf("%d ", sorted_items[i].id);
        }
    }
    printf("\nExecution Time: %.2f ms\n", time_taken);
    printf("Memory Usage: %zu bytes\n\n", max_memory_used);
    
    track_free(sorted_items, n * sizeof(Item));
    track_free(selected, n * sizeof(int));
    return time_taken;
}

// 回溯法辅助函数
void backtrack(Item *items, int n, int capacity, int depth, 
               int current_weight, double current_value,
               int *current_selection, int *best_selection, 
               double *best_value) {
    if (depth == n) {
        if (current_value > *best_value) {
            *best_value = current_value;
            memcpy(best_selection, current_selection, n * sizeof(int));
        }
        return;
    }
    
    // 选择当前物品
    if (current_weight + items[depth].weight <= capacity) {
        current_selection[depth] = 1;
        backtrack(items, n, capacity, depth + 1, 
                 current_weight + items[depth].weight, 
                 current_value + items[depth].value,
                 current_selection, best_selection, best_value);
        current_selection[depth] = 0;
    }
    
    // 不选择当前物品
    backtrack(items, n, capacity, depth + 1, 
             current_weight, current_value,
             current_selection, best_selection, best_value);
}

// 回溯法求解
double backtracking_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(Item) * n + 2 * sizeof(int) * n;
    
    int *current_selection = (int*)track_malloc(n * sizeof(int));
    int *best_selection = (int*)track_malloc(n * sizeof(int));
    double best_value = 0;
    
    if (!current_selection || !best_selection) {
        if (current_selection) track_free(current_selection, n * sizeof(int));
        if (best_selection) track_free(best_selection, n * sizeof(int));
        return -1;
    }
    
    memset(current_selection, 0, n * sizeof(int));
    memset(best_selection, 0, n * sizeof(int));
    
    backtrack(items, n, capacity, 0, 0, 0, 
             current_selection, best_selection, &best_value);
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("Backtracking Algorithm (Items=%d, Capacity=%d):\n", n, capacity);
    printf("Maximum Value: %.2f\n", best_value);
    printf("Selected Items: ");
    for (int i = 0; i < n; i++) {
        if (best_selection[i]) {
            printf("%d ", items[i].id);
        }
    }
    printf("\nExecution Time: %.2f ms\n", time_taken);
    printf("Memory Usage: %zu bytes\n\n", max_memory_used);
    
    track_free(current_selection, n * sizeof(int));
    track_free(best_selection, n * sizeof(int));
    return time_taken;
}

// 性能测试函数，返回测试结果
TestResult run_performance_test(int n, int capacity) {
    TestResult result;
    result.item_count = n;
    result.capacity = capacity;
    
    printf("==================== Performance Test ====================\n");
    printf("Number of Items: %d, Knapsack Capacity: %d\n\n", n, capacity);
    
    srand(time(NULL)); // 重置随机种子
    Item *items = generate_items(n);
    if (!items) {
        printf("Memory allocation failed\n");
        result.time.brute_force = -1;
        result.time.dynamic_programming = -1;
        result.time.greedy_selection = -1;
        result.time.greedy_quick = -1;
        result.time.backtracking = -1;
        return result;
    }
    
    // 当物品数量为1000且容量为100000时导出CSV
    if (n == 1000 && capacity == 100000) {
        export_to_csv(items, n, capacity);
    }
    
    // 初始化时间为-1表示未执行
    result.time.brute_force = -1;
    result.time.dynamic_programming = -1;
    result.time.greedy_selection = -1;
    result.time.greedy_quick = -1;
    result.time.backtracking = -1;
    
    // 仅在小规模时运行蛮力法和回溯法
    if (n <= 20) {
        result.time.brute_force = brute_force_knapsack(items, n, capacity);
        result.time.backtracking = backtracking_knapsack(items, n, capacity);
    }
    
    // 动态规划和贪心算法处理大规模数据
    if (n <= 320000 && capacity <= 1000000) {
        result.time.dynamic_programming = dp_knapsack(items, n, capacity);
        result.time.greedy_selection = greedy_selection_knapsack(items, n, capacity);
        result.time.greedy_quick = greedy_quick_knapsack(items, n, capacity);
    }
    
    track_free(items, n * sizeof(Item));
    return result;
}

// 将测试结果导出为表格
void export_to_table(TestResult *results, int count) {
    FILE *fp = fopen("knapsack_algorithm_performance.csv", "w");
    if (!fp) {
        printf("无法创建性能表格文件\n");
        return;
    }
    
    // 写入表格表头
    fprintf(fp, "Number of Items,Knapsack Capacity,Brute Force (ms),Dynamic Programming (ms),Greedy (Selection Sort) (ms),Greedy (Quick Sort) (ms),Backtracking (ms)\n");
    
    // 写入测试结果
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%d", 
                results[i].item_count, results[i].capacity);
        
        if (results[i].time.brute_force >= 0)
            fprintf(fp, ",%.2f", results[i].time.brute_force);
        else
            fprintf(fp, ",-");
        
        if (results[i].time.dynamic_programming >= 0)
            fprintf(fp, ",%.2f", results[i].time.dynamic_programming);
        else
            fprintf(fp, ",-");
        
        if (results[i].time.greedy_selection >= 0)
            fprintf(fp, ",%.2f", results[i].time.greedy_selection);
        else
            fprintf(fp, ",-");
        
        if (results[i].time.greedy_quick >= 0)
            fprintf(fp, ",%.2f", results[i].time.greedy_quick);
        else
            fprintf(fp, ",-");
        
        if (results[i].time.backtracking >= 0)
            fprintf(fp, ",%.2f", results[i].time.backtracking);
        else
            fprintf(fp, ",-");
        
        fprintf(fp, "\n");
    }
    
    fclose(fp);
    printf("性能测试结果已导出到: knapsack_algorithm_performance.csv\n");
}

int main() {
    printf("============= 0-1 Knapsack Problem Algorithm Performance Test =============\n");
    
    // 定义测试规模
    int item_counts[] = {10, 15, 20, 25, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 
                        10000, 20000, 40000, 80000};
    int capacities[] = {10000,  100000, 1000000}; 
    int num_counts = sizeof(item_counts) / sizeof(item_counts[0]);
    int num_capacities = sizeof(capacities) / sizeof(capacities[0]);
    
    // 计算测试用例总数
    int total_tests = 0;
    for (int i = 0; i < num_capacities; i++) {
        for (int j = 0; j < num_counts; j++) {
            // 跳过不合理的组合
            if (item_counts[j] > 20 && capacities[i] == 1000000) continue;
            total_tests++;
        }
    }
    
    // 分配测试结果数组
    TestResult *results = (TestResult*)track_malloc(total_tests * sizeof(TestResult));
    if (!results) {
        printf("内存分配失败\n");
        return 1;
    }
    
    // 运行性能测试并收集结果
    int test_idx = 0;
    for (int i = 0; i < num_capacities; i++) {
        for (int j = 0; j < num_counts; j++) {
            // 跳过不合理的组合
            if (item_counts[j] > 20 && capacities[i] == 1000000) continue;
            
            results[test_idx] = run_performance_test(item_counts[j], capacities[i]);
            test_idx++;
        }
    }
    
    // 导出测试结果为表格
    export_to_table(results, test_idx);
    
    printf("==================== Test Completed ====================\n");
    track_free(results, total_tests * sizeof(TestResult));
    return 0;
}
