#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <limits.h>

// ��Ʒ�ṹ��
typedef struct {
    int id;         // ��Ʒ���
    int weight;     // ��Ʒ����
    double value;   // ��Ʒ��ֵ
    double ratio;   // ��ֵ������
} Item;

// ȫ�ֱ������ڼ�¼�ڴ�ռ�÷�ֵ���㷨ִ��ʱ��
size_t max_memory_used = 0;
// �����㷨ִ��ʱ��洢�ṹ
typedef struct {
    double brute_force;
    double dynamic_programming;
    double greedy_selection;  // ԭѡ������̰���㷨
    double greedy_quick;      // ��������̰���㷨
    double backtracking;
} AlgorithmTime;

// ������Բ��������洢�ṹ
typedef struct {
    int item_count;
    int capacity;
    AlgorithmTime time;
} TestResult;

// �ڴ������ٺ���
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

// ���������Ʒ����
Item* generate_items(int n) {
    Item *items = (Item*)track_malloc(n * sizeof(Item));
    if (!items) return NULL;
    
    for (int i = 0; i < n; i++) {
        items[i].id = i + 1;
        items[i].weight = rand() % 100 + 1;  // 1~100���������
        items[i].value = (rand() % 9001) / 100.0 + 100; // 100.00~1000.00�������ֵ
        items[i].ratio = items[i].value / items[i].weight;
    }
    return items;
}

// �����Ʒ��Ϣ��CSV�ļ�
void export_to_csv(Item *items, int n, int capacity) {
    char filename[50];
    sprintf(filename, "��������%d_��Ʒ����%d.csv", capacity, n);
    
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("�޷�����CSV�ļ�\n");
        return;
    }
    
    // д���ͷ
    fprintf(fp, "��Ʒ���,��Ʒ����,��Ʒ��ֵ\n");
    
    // д����Ʒ����
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%.2f,%.2f\n", items[i].id, 
                (double)items[i].weight, items[i].value);
    }
    
    fclose(fp);
    printf("��Ʒ��Ϣ�ѵ�����: %s\n\n", filename);
}

// ���������
double brute_force_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(Item) * n; // �����ڴ�ͳ��
    
    double max_value = 0;
    int best_selection = 0;
    long total = (long)1 << n; // 2^n�ֿ�����
    
    // ���������Ӽ�
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
    
    // ������
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

// ��̬�滮�����
double dp_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(double) * (capacity + 1) + sizeof(Item) * n;
    
    // ����DP��
    double *dp = (double*)track_malloc((capacity + 1) * sizeof(double));
    if (!dp) return -1;
    
    // ��ʼ��DP��
    for (int w = 0; w <= capacity; w++) {
        dp[w] = 0;
    }
    
    // ��̬�滮���
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
    
    // ������
    printf("Dynamic Programming Algorithm (Items=%d, Capacity=%d):\n", n, capacity);
    printf("Maximum Value: %.2f\n", max_value);
    printf("Execution Time: %.2f ms\n", time_taken);
    printf("Memory Usage: %zu bytes\n\n", max_memory_used);
    
    track_free(dp, (capacity + 1) * sizeof(double));
    return time_taken;
}

// ��������ıȽϺ���
int compare_items(const void *a, const void *b) {
    Item *item1 = (Item *)a;
    Item *item2 = (Item *)b;
    if (item1->ratio < item2->ratio) return 1;   // ��������
    if (item1->ratio > item2->ratio) return -1;
    return 0;
}

// ѡ�������̰���㷨��ԭ�汾��
double greedy_selection_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(Item) * n + sizeof(int) * n;
    
    // ����ֵ����������ѡ������
    Item *sorted_items = (Item*)track_malloc(n * sizeof(Item));
    if (!sorted_items) return -1;
    memcpy(sorted_items, items, n * sizeof(Item));
    
    // ��ѡ������
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
    
    // ̰��ѡ��
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
    
    // ������
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

// ���������̰���㷨���°汾��
double greedy_quick_knapsack(Item *items, int n, int capacity) {
    clock_t start = clock();
    max_memory_used = sizeof(Item) * n + sizeof(int) * n;
    
    // ����ֵ���������򣨿�������
    Item *sorted_items = (Item*)track_malloc(n * sizeof(Item));
    if (!sorted_items) return -1;
    memcpy(sorted_items, items, n * sizeof(Item));
    
    // ʹ��qsort���п�������
    qsort(sorted_items, n, sizeof(Item), compare_items);
    
    // ̰��ѡ��
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
    
    // ������(Ӣ��)
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

// ���ݷ���������
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
    
    // ѡ��ǰ��Ʒ
    if (current_weight + items[depth].weight <= capacity) {
        current_selection[depth] = 1;
        backtrack(items, n, capacity, depth + 1, 
                 current_weight + items[depth].weight, 
                 current_value + items[depth].value,
                 current_selection, best_selection, best_value);
        current_selection[depth] = 0;
    }
    
    // ��ѡ��ǰ��Ʒ
    backtrack(items, n, capacity, depth + 1, 
             current_weight, current_value,
             current_selection, best_selection, best_value);
}

// ���ݷ����
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
    
    // ������
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

// ���ܲ��Ժ��������ز��Խ��
TestResult run_performance_test(int n, int capacity) {
    TestResult result;
    result.item_count = n;
    result.capacity = capacity;
    
    printf("==================== Performance Test ====================\n");
    printf("Number of Items: %d, Knapsack Capacity: %d\n\n", n, capacity);
    
    srand(time(NULL)); // �����������
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
    
    // ����Ʒ����Ϊ1000������Ϊ100000ʱ����CSV
    if (n == 1000 && capacity == 100000) {
        export_to_csv(items, n, capacity);
    }
    
    // ��ʼ��ʱ��Ϊ-1��ʾδִ��
    result.time.brute_force = -1;
    result.time.dynamic_programming = -1;
    result.time.greedy_selection = -1;
    result.time.greedy_quick = -1;
    result.time.backtracking = -1;
    
    // ����С��ģʱ�����������ͻ��ݷ�
    if (n <= 20) {
        result.time.brute_force = brute_force_knapsack(items, n, capacity);
        result.time.backtracking = backtracking_knapsack(items, n, capacity);
    }
    
    // ��̬�滮��̰���㷨������ģ����
    if (n <= 320000 && capacity <= 1000000) {
        result.time.dynamic_programming = dp_knapsack(items, n, capacity);
        result.time.greedy_selection = greedy_selection_knapsack(items, n, capacity);
        result.time.greedy_quick = greedy_quick_knapsack(items, n, capacity);
    }
    
    track_free(items, n * sizeof(Item));
    return result;
}

// �����Խ������Ϊ���
void export_to_table(TestResult *results, int count) {
    FILE *fp = fopen("knapsack_algorithm_performance.csv", "w");
    if (!fp) {
        printf("�޷��������ܱ���ļ�\n");
        return;
    }
    
    // д�����ͷ
    fprintf(fp, "Number of Items,Knapsack Capacity,Brute Force (ms),Dynamic Programming (ms),Greedy (Selection Sort) (ms),Greedy (Quick Sort) (ms),Backtracking (ms)\n");
    
    // д����Խ��
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
    printf("���ܲ��Խ���ѵ�����: knapsack_algorithm_performance.csv\n");
}

int main() {
    printf("============= 0-1 Knapsack Problem Algorithm Performance Test =============\n");
    
    // ������Թ�ģ
    int item_counts[] = {10, 15, 20, 25, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 
                        10000, 20000, 40000, 80000};
    int capacities[] = {10000,  100000, 1000000}; 
    int num_counts = sizeof(item_counts) / sizeof(item_counts[0]);
    int num_capacities = sizeof(capacities) / sizeof(capacities[0]);
    
    // ���������������
    int total_tests = 0;
    for (int i = 0; i < num_capacities; i++) {
        for (int j = 0; j < num_counts; j++) {
            // ��������������
            if (item_counts[j] > 20 && capacities[i] == 1000000) continue;
            total_tests++;
        }
    }
    
    // ������Խ������
    TestResult *results = (TestResult*)track_malloc(total_tests * sizeof(TestResult));
    if (!results) {
        printf("�ڴ����ʧ��\n");
        return 1;
    }
    
    // �������ܲ��Բ��ռ����
    int test_idx = 0;
    for (int i = 0; i < num_capacities; i++) {
        for (int j = 0; j < num_counts; j++) {
            // ��������������
            if (item_counts[j] > 20 && capacities[i] == 1000000) continue;
            
            results[test_idx] = run_performance_test(item_counts[j], capacities[i]);
            test_idx++;
        }
    }
    
    // �������Խ��Ϊ���
    export_to_table(results, test_idx);
    
    printf("==================== Test Completed ====================\n");
    track_free(results, total_tests * sizeof(TestResult));
    return 0;
}
