% MATLAB Script to Plot Knapsack Algorithm Performance from CSV (Optimized)
% IMPORTANT:
% 1. Save this .m file with UTF-8 encoding.
% 2. Ensure the file path DOES NOT contain Chinese characters or spaces.

% --- Configuration ---
csvFile = 'results.csv'; 
outputDir = 'performance_charts'; 
capacitiesToProcess = [10000, 100000, 1000000]; 

algorithms_internal = {'BruteForce', 'DP_2D', 'Greedy', 'Backtrack'};
algorithms_display = {'蛮力法', '动态规划法 (2D)', '贪心法', '回溯法'};

% 从C代码的CSV输出中获取实际使用的算法名称 (如果与algorithms_display不完全一致)
% 例如，如果CSV中DP是 "动态规划法-内存超限跳过" 或 "动态规划法 (2D)"
% 我们需要确保这里的名称与CSV中的主要名称匹配，不含 "-跳过" 后缀
csv_algo_names_map = containers.Map(...
    algorithms_internal,...
    {'蛮力法', '动态规划法 (2D)', '贪心法', '回溯法'} ... % 这些应与CSV中成功运行时的算法名称一致
); % CORRECTED: Added closing parenthesis for containers.Map here


if length(algorithms_internal) ~= length(algorithms_display)
    error('Internal and display algorithm name lists must have the same number of elements.');
end

algorithmColors = containers.Map(algorithms_internal, {'r', 'g', 'b', 'm'});
algorithmMarkers = containers.Map(algorithms_internal, {'o', 's', 'x', '^'});

% --- Script Start ---
if ~exist(outputDir, 'dir')
   mkdir(outputDir);
end

try
    opts = detectImportOptions(csvFile, 'Encoding', 'UTF-8');
    % 指定TimeMS为double，以防被错误解析为文本（如果CSV中有非数值）
    opts = setvartype(opts, 'TimeMS', 'double');
    opts = setvartype(opts, 'ItemCount', 'double');
    opts = setvartype(opts, 'Capacity', 'double');
    opts = setvartype(opts, 'TotalValue', 'double');
    opts = setvartype(opts, 'TotalWeight_Selected', 'double');
    dataTable = readtable(csvFile, opts);
catch ME
    error('Failed to read CSV file "%s": %s\nPlease ensure UTF-8 encoding and correct format.', csvFile, ME.message);
end

requiredColumns = {'Algorithm', 'ItemCount', 'Capacity', 'TimeMS'};
if ~all(ismember(requiredColumns, dataTable.Properties.VariableNames))
    error('CSV file is missing required columns. Needed: %s. Found: %s', strjoin(requiredColumns, ', '), strjoin(dataTable.Properties.VariableNames, ', '));
end

minNonZeroTime = 0.001; % 用于替换0ms的值，以便在对数刻度上显示

for cap_idx = 1:length(capacitiesToProcess)
    currentCapacity = capacitiesToProcess(cap_idx);
    
    figureHandle = figure('Name', sprintf('性能对比 (C = %d)', currentCapacity), 'NumberTitle', 'off', 'Position', [100, 100, 1000, 700]);
    ax = gca; % Get current axes
    hold(ax, 'on');
    grid(ax, 'on');
    
    plotHandles = [];
    legendTexts = {}; % Not strictly needed if using DisplayName in plot directly for legend
    
    allItemCounts = []; % 用于确定X轴范围

    for algo_idx = 1:length(algorithms_internal)
        currentAlgorithmInternalName = algorithms_internal{algo_idx};
        currentAlgorithmDisplayName = algorithms_display{algo_idx};
        
        % 检查 csv_algo_names_map 是否包含当前 internal name 作为键
        if ~isKey(csv_algo_names_map, currentAlgorithmInternalName)
            fprintf('Warning: Algorithm name "%s" not found in csv_algo_names_map. Skipping.\n', currentAlgorithmInternalName);
            continue;
        end
        csvAlgorithmName = csv_algo_names_map(currentAlgorithmInternalName); % 获取CSV中对应的算法名

        % 筛选当前容量和当前算法的数据 (只选择成功运行的条目)
        try
            selectedRows = dataTable.Capacity == currentCapacity & ...
                           strcmp(dataTable.Algorithm, csvAlgorithmName); % 精确匹配CSV中的算法名
            algoData = dataTable(selectedRows, :);
        catch ME_filter
            fprintf('Warning: Could not filter data for algorithm "%s" (CSV name: "%s") at capacity %d. Error: %s\n', ...
                    currentAlgorithmDisplayName, csvAlgorithmName, currentCapacity, ME_filter.message);
            algoData = [];
        end
        
        if isempty(algoData) || height(algoData) < 2 % 需要至少2个点才能画线
            fprintf('Info: Not enough valid data points (<2) to plot for algorithm "%s" at capacity %d.\n', currentAlgorithmDisplayName, currentCapacity);
            % 检查是否有跳过记录，并在图例中说明
            % 注意: contains(dataTable.Algorithm, csvAlgorithmName) 可能会匹配到 "动态规划法 (2D)" 和 "动态规划法-内存超限跳过"
            % 更精确的匹配跳过条目：
            skipped_entry_pattern = [csvAlgorithmName, '-']; % 例如 "蛮力法-"
            skippedRows = dataTable.Capacity == currentCapacity & startsWith(dataTable.Algorithm, skipped_entry_pattern);

            if any(skippedRows) % if ~isempty(skipped_entry)
                 % 创建一个不可见的绘图对象仅用于图例条目
                 h_skip = plot(ax, NaN, NaN, 'Marker', 'none', 'LineStyle', 'none', ... 
                                'DisplayName', [currentAlgorithmDisplayName ' (数据不足/跳过)']);
                 
                 % 避免重复添加相同的“跳过”图例条目
                 is_already_in_legend = false;
                 for k_leg = 1:length(plotHandles)
                     if strcmp(get(plotHandles(k_leg),'DisplayName'), get(h_skip,'DisplayName'))
                         is_already_in_legend = true;
                         delete(h_skip); % 删除多余的不可见绘图对象
                         break;
                     end
                 end
                 if ~is_already_in_legend
                    plotHandles(end+1) = h_skip;
                 end
            end
            continue; 
        end
        
        algoData = sortrows(algoData, 'ItemCount');
        itemCounts = algoData.ItemCount;
        executionTimes = algoData.TimeMS;
        
        allItemCounts = unique([allItemCounts; itemCounts]); % 收集所有N值

        % 替换0或负时间为minNonZeroTime
        plotExecutionTimes = executionTimes;
        zeroOrNegativeIdx = plotExecutionTimes <= 1e-9; % 使用一个小的epsilon比较浮点数
        if any(zeroOrNegativeIdx)
             plotExecutionTimes(zeroOrNegativeIdx) = minNonZeroTime; 
             fprintf('Info: Replaced <=0ms execution times with %.3fms for log scale plotting for "%s" at C=%d.\n', minNonZeroTime, currentAlgorithmDisplayName, currentCapacity);
        end

        if isKey(algorithmMarkers, currentAlgorithmInternalName) && isKey(algorithmColors, currentAlgorithmInternalName)
            h = loglog(ax, itemCounts, plotExecutionTimes, ...
                 'LineWidth', 1.5, ...
                 'Marker', algorithmMarkers(currentAlgorithmInternalName), ...
                 'MarkerSize', 8, ...
                 'Color', algorithmColors(currentAlgorithmInternalName), ...
                 'DisplayName', currentAlgorithmDisplayName); % DisplayName 用于图例
            plotHandles(end+1) = h;
            % legendTexts is not strictly needed if DisplayName is set directly
        else
            fprintf('Warning: Marker/Color key not found for algorithm: %s.\n', currentAlgorithmInternalName);
        end
    end
    
    hold(ax, 'off');
    
    titleStr = sprintf('0-1背包算法执行时间对比 (容量 C = %d)', currentCapacity);
    title(ax, titleStr, 'FontSize', 14, 'Interpreter', 'none'); 
    xlabel(ax, '物品数量 N (对数刻度)', 'FontSize', 12);
    ylabel(ax, '执行时间 T(N) (ms, 对数刻度)', 'FontSize', 12);
    
    if ~isempty(plotHandles)
        legend(ax, plotHandles, 'Location', 'northwest', 'FontSize', 10, 'Interpreter', 'none'); 
    else
        text(ax, 0.5, 0.5, '没有足够的数据用于此容量下的图表绘制', ...
             'HorizontalAlignment', 'center', 'Units', 'normalized', 'FontSize', 12, 'Color', 'red');
        fprintf('Warning: No data plotted for capacity %d.\n', currentCapacity);
    end
    
    set(ax, 'XScale', 'log', 'YScale', 'log');
    
    % 动态调整X轴和Y轴范围以更好地显示数据
    if ~isempty(allItemCounts)
        minN_val = min(allItemCounts(allItemCounts>0)); % Ensure minN_val is positive
        maxN_val = max(allItemCounts);
        if ~isempty(minN_val) && ~isempty(maxN_val) && maxN_val > minN_val
            xlim(ax, [max(1, minN_val*0.8), maxN_val*1.2]); 
        elseif ~isempty(minN_val) 
             xlim(ax, [max(1,minN_val*0.8), minN_val*1.2]);
        end
    end
    
    y_data_all = [];
    children = get(ax, 'Children');
    for k_child = 1:length(children)
        if isa(children(k_child), 'matlab.graphics.chart.primitive.Line') % Check if it's a line object
            y_data_child = get(children(k_child), 'YData');
            y_data_all = [y_data_all, y_data_child(y_data_child > minNonZeroTime/2)]; % Collect data slightly above minNonZeroTime
        end
    end

    if ~isempty(y_data_all)
        minT_val = min(y_data_all);
        maxT_val = max(y_data_all);
        if minT_val > 0 && maxT_val > minT_val
             ylim(ax, [minT_val*0.5, maxT_val*2]); 
        elseif minT_val > 0 
             ylim(ax, [minT_val*0.5, minT_val*2]);
        else 
             ylim(ax, [minNonZeroTime*0.5, minNonZeroTime*100]); % Adjusted default if only minNonZeroTime points
        end
    else 
        ylim(ax, [minNonZeroTime, 1000]);
    end

    chartFilename = fullfile(outputDir, sprintf('performance_C%d.png', currentCapacity));
    try
        saveas(figureHandle, chartFilename); 
        fprintf('图表已保存为: %s\n', chartFilename);
    catch ME_save
        fprintf('Warning: Could not save chart "%s": %s\n', chartFilename, ME_save.message);
    end
end

fprintf('所有图表处理完毕。\n');
