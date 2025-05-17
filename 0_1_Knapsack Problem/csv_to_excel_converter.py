import pandas as pd
import os # 导入os模块

def convert_csv_to_excel(csv_file_path, excel_file_path):
    """
    将指定的CSV文件转换为Excel (.xlsx) 文件。

    参数:
    csv_file_path (str): 输入的CSV文件的路径。
    excel_file_path (str): 输出的Excel文件的路径。
    """
    try:
        # 检查输入文件是否存在
        if not os.path.exists(csv_file_path):
            print(f"错误：输入文件 '{csv_file_path}' 未找到。请检查文件路径是否正确。")
            return

        # 读取CSV文件到pandas DataFrame
        # 假设CSV文件使用UTF-8编码
        df = pd.read_csv(csv_file_path, encoding='utf-8')

        # 将DataFrame写入Excel文件
        # index=False 表示不将DataFrame的索引写入Excel文件
        # 根据输出文件名决定工作表名称
        if 'results' in os.path.basename(excel_file_path).lower():
             sheet_name = 'PerformanceResults' # 算法性能数据的工作表名
        else:
             sheet_name = 'Items_N1000_Data' # N=1000物品清单数据的工作表名

        df.to_excel(excel_file_path, index=False, sheet_name=sheet_name)

        print(f"文件 '{csv_file_path}' 已成功转换为 '{excel_file_path}'")

    except FileNotFoundError:
        print(f"错误：未找到文件 '{csv_file_path}'。")
    except Exception as e:
        print(f"转换过程中发生错误：{e}")

if __name__ == '__main__':
    # 获取脚本所在的目录
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # --- 转换算法性能结果 'results.csv' ---
    input_csv_file_name = 'results.csv'
    input_csv_file_path = os.path.join(script_dir, input_csv_file_name)

    output_excel_file_name = 'results.xlsx' # 这个文件主要用于您自己查看或备份性能数据
    output_excel_file_path = os.path.join(script_dir, output_excel_file_name)

    print(f"脚本所在目录: {script_dir}")
    if os.path.exists(input_csv_file_path):
        print(f"准备转换算法性能文件: {input_csv_file_path} -> {output_excel_file_path}")
        convert_csv_to_excel(input_csv_file_path, output_excel_file_path)
    else:
        print(f"提示: 算法性能文件 '{input_csv_file_path}' 未找到。")


    # --- 为实验报告中要求的“学号-姓名-数据.xlsx”准备 ---
    # (包含1000个物品时的物品编号、物品重量、物品价值)
    # 假设您的C代码生成了一个名为 'items_n1000_for_report.csv' 的文件
    # 这个CSV文件应该只包含N=1000时，某一特定容量（例如C=10000）下的物品具体清单
    input_items_data_csv_name = 'items_n1000_for_report.csv' # **重要**: 请确保C代码生成此文件或类似文件
    input_items_data_csv_path = os.path.join(script_dir, input_items_data_csv_name)

    # 实验指导要求的Excel文件名格式
    student_id = "20231060285" # 已更新为您的学号
    student_name = "彭家城"   # 已更新为您的姓名
    output_items_data_excel_name = f"{student_id}-{student_name}-数据.xlsx" # 符合要求的Excel文件名
    output_items_data_excel_path = os.path.join(script_dir, output_items_data_excel_name)

    print(f"\n如果您还需要转换用于实验报告附件的N=1000物品清单数据：")
    if os.path.exists(input_items_data_csv_path):
        print(f"准备转换物品清单文件: {input_items_data_csv_path} -> {output_items_data_excel_path}")
        convert_csv_to_excel(input_items_data_csv_path, output_items_data_excel_path)
    else:
        print(f"提示: 特定物品清单文件 '{input_items_data_csv_path}' 未找到。")
        print(f"请确保您的C程序已在脚本目录生成此文件（例如，只包含N=1000时C=10000下的物品编号、重量、价值）。")
        print(f"或者，请修改脚本中的 '{input_items_data_csv_name}' 为您C代码实际生成的对应文件名。")
        print(f"最终生成的Excel文件名将是 '{output_items_data_excel_name}'。")

