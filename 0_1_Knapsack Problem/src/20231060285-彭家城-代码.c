#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> // For memset, memcpy
#include <math.h>   // For pow, round
#include <float.h>  // For DBL_MAX

// 定义物品结构体
typedef struct {
    int id;           // 物品在当前算法内部处理时的标识 (例如排序后的索引+1)
    int original_id;  // 物品在原始输入数据中的编号 (从1开始)
    double w_double;  // 物品的原始浮点数重量
    int w_int_scaled; // 物品重量乘以100后的整数值 (用于DP)
    int v;            // 物品价值 (存储为乘以100后的整数，以表示两位小数)
    double ratio;     // 价值/重量比 (v_double / w_double)，用于贪心算法和回溯法剪枝
} Item;

// CSV文件指针
FILE* csv_fp = NULL;

// 函数：记录结果到CSV文件
// 注意: total_value 现在是缩放后的整数总价值
void log_result_to_csv(const char* algorithm_name, int n, int capacity_original, int total_value_scaled, double total_weight_selected, double time_taken) {
    if (csv_fp != NULL) {
        // 输出时可以将 total_value_scaled 转换为原始两位小数形式
        fprintf(csv_fp, "%s,%d,%d,%.2f,%.2f,%.2f\n", algorithm_name, n, capacity_original, (double)total_value_scaled / 100.0, total_weight_selected, time_taken);
    }
}

// 辅助函数：打印选中的物品信息 (通用详细打印)
// selected_flags 对应 original_items_input 的顺序
// calculated_max_value_scaled 是缩放后的最大价值
void print_selected_items_detailed(const char* algorithm_name_display, int n, const Item original_items_input[], const int selected_flags[], int calculated_max_value_scaled, double total_weight_selected_double, double time_taken) {
    printf("\n--- %s ---\n", algorithm_name_display);
    int items_count = 0;
    for (int i = 0; i < n; ++i) {
        if (selected_flags[i] == 1) {
            items_count++;
        }
    }

    if (items_count == 0 && calculated_max_value_scaled > 0) {
        printf("  警告: 计算得到的总价值为 %.2f (缩放前)，但没有标记任何选中物品。\n", (double)calculated_max_value_scaled / 100.0);
    } else if (items_count == 0) {
        printf("  没有选择任何物品。\n");
    }else {
        printf("选中的物品 (基于原始编号):\n");
        for (int i = 0; i < n; ++i) {
            if (selected_flags[i] == 1) {
                // 打印时将缩放的价值转换回两位小数形式
                printf("  物品 (原始编号 %d): 重量 %.2f, 价值 %.2f\n", original_items_input[i].original_id, original_items_input[i].w_double, (double)original_items_input[i].v / 100.0);
            }
        }
    }
    // 打印时将缩放的总价值转换回两位小数形式
    printf("总价值 (算法计算): %.2f\n", (double)calculated_max_value_scaled / 100.0);
    printf("总重量 (浮点): %.2f\n", total_weight_selected_double);
    printf("执行时间: %.2f ms\n", time_taken);
    printf("--------------------------\n");
}


// 1. 蛮力法 (Brute Force)
// max_value 和 current_value 现在处理的是缩放后的整数价值
void brute_force(int n, const Item original_items_input[], int capacity_original) {
    if (n > 15) {
        printf("\n--- 蛮力法 ---\n");
        printf("物品数量 %d 过大 (阈值15)，跳过蛮力法测试以节省时间。\n", n);
        printf("--------------------------\n");
        log_result_to_csv("蛮力法-跳过", n, capacity_original, 0, 0.0, 0.0);
        return;
    }
    clock_t start_time = clock();

    int max_value_scaled = 0; // 存储缩放后的最大价值
    double max_weight_at_max_value = 0.0;
    int* best_selection_flags = (int*)calloc(n, sizeof(int));
    if (!best_selection_flags) {
        perror("Failed to allocate memory for best_selection_flags in brute_force");
        return;
    }

    long long num_combinations = 1LL << n;

    for (long long i = 0; i < num_combinations; ++i) {
        double current_weight_double = 0;
        int current_value_scaled = 0; // 存储当前组合的缩放后价值
        int* current_selection_flags = (int*)calloc(n, sizeof(int));
        if (!current_selection_flags) {
            perror("Failed to allocate memory for current_selection_flags in brute_force loop");
            free(best_selection_flags);
            return;
        }

        for (int j = 0; j < n; ++j) {
            if ((i >> j) & 1) {
                current_weight_double += original_items_input[j].w_double;
                current_value_scaled += original_items_input[j].v; // v已经是缩放后的整数
                current_selection_flags[j] = 1;
            }
        }

        if (current_weight_double <= capacity_original && current_value_scaled > max_value_scaled) {
            max_value_scaled = current_value_scaled;
            max_weight_at_max_value = current_weight_double;
            memcpy(best_selection_flags, current_selection_flags, n * sizeof(int));
        }
        free(current_selection_flags);
    }

    double time_taken = ((double)(clock() - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    log_result_to_csv("蛮力法", n, capacity_original, max_value_scaled, max_weight_at_max_value, time_taken);
    print_selected_items_detailed("蛮力法", n, original_items_input, best_selection_flags, max_value_scaled, max_weight_at_max_value, time_taken);

    free(best_selection_flags);
}

// 2. 动态规划法 (Dynamic Programming)
// DP表直接使用缩放后的整数价值 original_items_input[i-1].v
void dynamic_programming(int n, const Item original_items_input[], int capacity_original) {
    clock_t start_time = clock();
    int capacity_scaled = capacity_original * 100;
    const long long MAX_DP_MEMORY_BYTES = 2LL * 1024 * 1024 * 1024;
    long long estimated_elements = (long long)(n + 1) * (capacity_scaled + 1);
    long long estimated_memory_needed = estimated_elements * sizeof(int);

    if (estimated_memory_needed > MAX_DP_MEMORY_BYTES) {
        printf("\n--- 动态规划法 (2D) ---\n");
        printf("警告: 物品数量 %d, 背包容量 %d (缩放后 %d) 将导致DP表过大 (预估 %.2f MB)，超过 %.0f MB 限制，跳过此测试。\n",
               n, capacity_original, capacity_scaled, (double)estimated_memory_needed / (1024*1024), (double)MAX_DP_MEMORY_BYTES / (1024*1024));
        printf("--------------------------\n");
        log_result_to_csv("动态规划法-内存超限跳过", n, capacity_original, 0, 0.0, 0.0);
        return;
    }

    int** dp = (int**)malloc((n + 1) * sizeof(int*));
    if (!dp) { /* ... error handling ... */ return; }
    for (int i = 0; i <= n; ++i) {
        dp[i] = (int*)calloc(capacity_scaled + 1, sizeof(int));
        if (!dp[i]) { /* ... error handling ... */ return; }
    }

    for (int i = 1; i <= n; ++i) {
        int item_weight_scaled = original_items_input[i-1].w_int_scaled;
        int item_value_scaled = original_items_input[i-1].v; // v已经是缩放后的整数
        for (int j = 0; j <= capacity_scaled; ++j) {
            if (item_weight_scaled <= j) {
                if (dp[i-1][j - item_weight_scaled] + item_value_scaled > dp[i-1][j]) {
                    dp[i][j] = dp[i-1][j - item_weight_scaled] + item_value_scaled;
                } else {
                    dp[i][j] = dp[i-1][j];
                }
            } else {
                dp[i][j] = dp[i-1][j];
            }
        }
    }

    int final_max_value_scaled = dp[n][capacity_scaled];
    int* selected_flags = (int*)calloc(n, sizeof(int));
    if (!selected_flags) { /* ... error handling ... */ return; }

    int current_scaled_capacity_trace = capacity_scaled;
    for (int i = n; i > 0 && current_scaled_capacity_trace > 0; --i) {
        if (dp[i][current_scaled_capacity_trace] != dp[i-1][current_scaled_capacity_trace]) {
            selected_flags[i-1] = 1;
            current_scaled_capacity_trace -= original_items_input[i-1].w_int_scaled;
        }
    }

    double total_weight_selected_double = 0;
    for(int i=0; i<n; ++i) {
        if(selected_flags[i] == 1) {
            total_weight_selected_double += original_items_input[i].w_double;
        }
    }
    double time_taken = ((double)(clock() - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    log_result_to_csv("动态规划法 (2D)", n, capacity_original, final_max_value_scaled, total_weight_selected_double, time_taken);
    print_selected_items_detailed("动态规划法 (2D)", n, original_items_input, selected_flags, final_max_value_scaled, total_weight_selected_double, time_taken);

    for (int i = 0; i <= n; ++i) free(dp[i]);
    free(dp);
    free(selected_flags);
}


// 辅助函数：用于qsort的比较函数（按价值/重量比降序）
// ratio is (v_scaled / 100.0) / w_double
int compare_items_ratio_desc(const void* a, const void* b) {
    Item* item_a = (Item*)a;
    Item* item_b = (Item*)b;
    // ratio is calculated on the fly or pre-calculated based on original double values
    if (item_a->ratio < item_b->ratio) return 1;
    if (item_a->ratio > item_b->ratio) return -1;
    if (item_a->w_double < item_b->w_double) return -1;
    if (item_a->w_double > item_b->w_double) return 1;
    return 0;
}

// 3. 贪心法 (Greedy Algorithm)
// total_value_greedy_scaled 现在处理的是缩放后的整数价值
void greedy_algorithm(int n, const Item original_items_input[], int capacity_original) {
    clock_t start_time = clock();
    Item* items_to_sort = (Item*)malloc(n * sizeof(Item));
    if (!items_to_sort) { /* ... error handling ... */ return; }
    memcpy(items_to_sort, original_items_input, n * sizeof(Item));

    // Ratio calculation remains the same as it uses original_items_input[i].v (scaled) and w_double
    // The Item struct's ratio field should be calculated based on the conceptual float value.
    // This was already done in generate_random_items after v was set.

    qsort(items_to_sort, n, sizeof(Item), compare_items_ratio_desc);

    int* selected_flags_original_order = (int*)calloc(n, sizeof(int));
    if (!selected_flags_original_order) { /* ... error handling ... */ free(items_to_sort); return; }

    double current_weight_double_greedy = 0;
    int total_value_greedy_scaled = 0; // 存储缩放后的总价值
    int total_selected_count_greedy = 0;

    for (int i = 0; i < n; ++i) {
        if (current_weight_double_greedy + items_to_sort[i].w_double <= capacity_original) {
            selected_flags_original_order[items_to_sort[i].original_id - 1] = 1;
            current_weight_double_greedy += items_to_sort[i].w_double;
            total_value_greedy_scaled += items_to_sort[i].v; // v已经是缩放后的整数
            total_selected_count_greedy++;
        }
    }
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    log_result_to_csv("贪心法", n, capacity_original, total_value_greedy_scaled, current_weight_double_greedy, time_taken);
    // print_selected_items_detailed will handle displaying the value correctly
    print_selected_items_detailed("贪心法", n, original_items_input, selected_flags_original_order, total_value_greedy_scaled, current_weight_double_greedy, time_taken);


    free(items_to_sort);
    free(selected_flags_original_order);
}

// --- 回溯法辅助结构和函数 ---
typedef struct {
    int max_value_scaled; // 存储缩放后的最大价值
    double weight_at_max_value;
    int* best_selection_flags_sorted_order;
} BacktrackResult;

// get_upper_bound_backtrack: current_v_scaled is scaled, sorted_items[i].v is scaled
// ratio is (v_scaled/100.0)/w_double, so bound calculation needs care if mixing scaled and unscaled
// For simplicity, let's assume the bound also works with scaled values if current_v_scaled is scaled.
// The ratio in Item struct is already (conceptual_float_value / w_double).
// So, if current_v_scaled is used, the bound should also be scaled.
// Let's adjust get_upper_bound to work with scaled values consistently.
double get_upper_bound_backtrack_scaled(int k_item_idx, int n, double current_w, int current_v_scaled, int capacity, const Item sorted_items[]) {
    double bound_scaled = current_v_scaled; // Start with current scaled value
    double remaining_capacity = capacity - current_w;

    for (int i = k_item_idx; i < n; ++i) {
        if (sorted_items[i].w_double <= remaining_capacity) {
            remaining_capacity -= sorted_items[i].w_double;
            bound_scaled += sorted_items[i].v; // Add scaled value
        } else {
            // sorted_items[i].ratio is (conceptual_v_float / w_double)
            // To get scaled bound contribution: ratio * remaining_capacity * 100
            bound_scaled += sorted_items[i].ratio * remaining_capacity * 100.0;
            break;
        }
    }
    return bound_scaled;
}


void backtrack_recursive(int k_item_idx, int n, double current_w, int current_v_scaled,
                         const Item sorted_items[], int capacity,
                         int* current_selection_flags_sorted, BacktrackResult* result) {
    if (k_item_idx == n) {
        if (current_v_scaled > result->max_value_scaled) {
            result->max_value_scaled = current_v_scaled;
            result->weight_at_max_value = current_w;
            memcpy(result->best_selection_flags_sorted_order, current_selection_flags_sorted, n * sizeof(int));
        }
        return;
    }

    if (get_upper_bound_backtrack_scaled(k_item_idx, n, current_w, current_v_scaled, capacity, sorted_items) <= result->max_value_scaled) {
        return;
    }

    current_selection_flags_sorted[k_item_idx] = 0;
    backtrack_recursive(k_item_idx + 1, n, current_w, current_v_scaled, sorted_items, capacity, current_selection_flags_sorted, result);

    if (current_w + sorted_items[k_item_idx].w_double <= capacity) {
        current_selection_flags_sorted[k_item_idx] = 1;
        backtrack_recursive(k_item_idx + 1, n,
                            current_w + sorted_items[k_item_idx].w_double,
                            current_v_scaled + sorted_items[k_item_idx].v, // Add scaled value
                            sorted_items, capacity, current_selection_flags_sorted, result);
        current_selection_flags_sorted[k_item_idx] = 0;
    }
}

// 4. 回溯法 (Backtracking)
// result.max_value_scaled stores scaled value
void backtracking(int n_input, const Item original_items_input[], int capacity_original) {
     if (n_input > 32) {
        printf("\n--- 回溯法 ---\n");
        printf("物品数量 %d 过大 (阈值32)，跳过回溯法测试以节省时间。\n", n_input);
        printf("--------------------------\n");
        log_result_to_csv("回溯法-跳过", n_input, capacity_original, 0, 0.0, 0.0);
        return;
    }
    clock_t start_time = clock();
    Item* items_to_sort = (Item*)malloc(n_input * sizeof(Item));
    if (!items_to_sort) { /* ... error handling ... */ return; }
    memcpy(items_to_sort, original_items_input, n_input * sizeof(Item));

    // Ratio calculation in items_to_sort is based on its own .v (scaled) and .w_double
    // This was already handled by generate_random_items which sets item.ratio
    qsort(items_to_sort, n_input, sizeof(Item), compare_items_ratio_desc);

    BacktrackResult result;
    result.max_value_scaled = 0; // Initialize scaled max value
    result.weight_at_max_value = 0.0;
    result.best_selection_flags_sorted_order = (int*)calloc(n_input, sizeof(int));
    int* current_selection_flags_sorted = (int*)calloc(n_input, sizeof(int));

    if (!result.best_selection_flags_sorted_order || !current_selection_flags_sorted) { /* ... error handling ... */ return; }

    backtrack_recursive(0, n_input, 0.0, 0, items_to_sort, capacity_original, current_selection_flags_sorted, &result);

    int* final_selected_flags_original_order = (int*)calloc(n_input, sizeof(int));
    if (!final_selected_flags_original_order) { /* ... error handling ... */ return; }

    double actual_weight_backtracking = 0.0;
    for (int i = 0; i < n_input; ++i) {
        if (result.best_selection_flags_sorted_order[i] == 1) {
            final_selected_flags_original_order[items_to_sort[i].original_id - 1] = 1;
            actual_weight_backtracking += items_to_sort[i].w_double;
        }
    }

    double time_taken = ((double)(clock() - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    log_result_to_csv("回溯法", n_input, capacity_original, result.max_value_scaled, actual_weight_backtracking, time_taken);
    print_selected_items_detailed("回溯法", n_input, original_items_input, final_selected_flags_original_order, result.max_value_scaled, actual_weight_backtracking, time_taken);

    free(items_to_sort);
    free(result.best_selection_flags_sorted_order);
    free(current_selection_flags_sorted);
    free(final_selected_flags_original_order);
}

// 数据生成函数
void generate_random_items(int n, Item items_array[]) {
    for (int i = 0; i < n; ++i) {
        items_array[i].original_id = i + 1;

        double raw_weight = (double)rand() / RAND_MAX * 99.0 + 1.0; // 1.0 to 100.0
        items_array[i].w_double = round(raw_weight * 100.0) / 100.0;
        if (items_array[i].w_double < 1.00) items_array[i].w_double = 1.00;
        if (items_array[i].w_double > 100.00) items_array[i].w_double = 100.00;
        items_array[i].w_int_scaled = (int)(items_array[i].w_double * 100.0 + 0.5);

        // MODIFICATION: Generate value as integer representing value*100 (100.00 to 1000.00)
        // rand() % 90001 generates 0-90000. Add 10000 to make it 10000-100000.
        // This corresponds to 100.00 to 1000.00 when divided by 100.
        items_array[i].v = rand() % 90001 + 10000; // v is now scaled, e.g., 12345 for 123.45

        // Calculate ratio using conceptual float value and double weight for better precision in greedy/backtrack
        if (items_array[i].w_double > 1e-9) {
            items_array[i].ratio = ((double)items_array[i].v / 100.0) / items_array[i].w_double;
        } else {
            items_array[i].ratio = (items_array[i].v > 0) ? DBL_MAX : 0;
        }
    }
}

// 主函数：用于测试
int main() {
    srand(time(NULL));

    csv_fp = fopen("results.csv", "w");
    if (csv_fp == NULL) { /* ... error handling ... */ return 1; }
    fprintf(csv_fp, "Algorithm,ItemCount,Capacity,TotalValue,TotalWeight_Selected,TimeMS\n"); // TotalValue here will be float

    const int capacities[] = {10000, 100000, 1000000};
    const int item_counts_example[] = {
        4, 10, 15, 20, 25, 30, 32,
        35, 40, 45, 50, 60, 70, 80, 90, 100,
        150, 200, 250, 300, 350, 400, 450, 500,
        750, 1000, 1500, 2000, 3000, 4000, 5000
    };

    int num_capacities = sizeof(capacities) / sizeof(capacities[0]);
    int num_item_counts = sizeof(item_counts_example) / sizeof(item_counts_example[0]);

    printf("--- 小型固定数据测试 (n=4, c=7) ---\n");
    Item fixed_items[4];
    double w_example_double[] = {2.0, 3.0, 4.0, 5.0};
    int v_example_scaled[] = {300, 400, 500, 600}; // Values scaled by 100 (3.00, 4.00, 5.00, 6.00)
    for(int i=0; i<4; ++i) {
        fixed_items[i].original_id = i+1;
        fixed_items[i].w_double = w_example_double[i];
        fixed_items[i].w_int_scaled = (int)(w_example_double[i] * 100 + 0.5);
        fixed_items[i].v = v_example_scaled[i]; // Assign scaled value
        if (fixed_items[i].w_double > 1e-9) {
            fixed_items[i].ratio = ((double)fixed_items[i].v / 100.0) / fixed_items[i].w_double;
        } else {
            fixed_items[i].ratio = fixed_items[i].v > 0 ? DBL_MAX : 0;
        }
    }
    int c_example = 7;
    brute_force(4, fixed_items, c_example);
    dynamic_programming(4, fixed_items, c_example);
    greedy_algorithm(4, fixed_items, c_example);
    backtracking(4, fixed_items, c_example);
    printf("--- 小型固定数据测试结束 ---\n\n");

    Item* items = NULL;
    int max_n_for_items_array = 0;
    for(int n_idx = 0; n_idx < num_item_counts; ++n_idx) {
        if (item_counts_example[n_idx] > max_n_for_items_array) {
            max_n_for_items_array = item_counts_example[n_idx];
        }
    }
    if (max_n_for_items_array > 0) {
        items = (Item*)malloc(max_n_for_items_array * sizeof(Item));
        if (!items) { /* ... error handling ... */ return 1; }
    }

    int n1000_c10000_items_logged = 0;

    for (int n_idx = 0; n_idx < num_item_counts; ++n_idx) {
        int current_n = item_counts_example[n_idx];
        if (items == NULL && current_n > 0) { /* ... error handling ... */ return 1; }
        if (current_n > 0) {
            generate_random_items(current_n, items);
        }

        for (int cap_idx = 0; cap_idx < num_capacities; ++cap_idx) {
            int current_capacity = capacities[cap_idx];

            if (current_n == 1000 && current_capacity == 10000 && !n1000_c10000_items_logged) {
                FILE* items_report_fp = fopen("items_n1000_for_report.csv", "w");
                if (items_report_fp == NULL) {
                    perror("Failed to open items_n1000_for_report.csv for writing");
                } else {
                    fprintf(items_report_fp, "物品编号,物品重量,物品价值\n");
                    for (int i = 0; i < current_n; ++i) {
                        fprintf(items_report_fp, "%d,%.2f,%.2f\n", // Output value as float with 2 decimal places
                                items[i].original_id,
                                items[i].w_double,
                                (double)items[i].v / 100.0); // Convert scaled int value back to float
                    }
                    fclose(items_report_fp);
                    printf("\n>>> N=1000, C=10000 的物品数据已记录到 items_n1000_for_report.csv <<<\n");
                    n1000_c10000_items_logged = 1;
                }
            }

            if (current_n == 4 && current_capacity == c_example) { continue; }
            if (current_n == 0) continue;

            printf("\n====================================================\n");
            printf("测试数据: 物品数量 n = %d, 背包容量 c = %d\n", current_n, current_capacity);
            printf("====================================================\n");

            brute_force(current_n, items, current_capacity);
            dynamic_programming(current_n, items, current_capacity);
            greedy_algorithm(current_n, items, current_capacity);
            backtracking(current_n, items, current_capacity);
        }
    }

    if (items != NULL) { free(items); }
    if (csv_fp != NULL) { fclose(csv_fp); }
    printf("\n实验结果已记录到 results.csv\n");

    return 0;
}
