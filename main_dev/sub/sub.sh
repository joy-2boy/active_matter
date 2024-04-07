#!/bin/bash

# 定义提交函数
# 参数位置 1:jobpath 2:N 3:T 4:t_p 5:phi 6:f 7: seed 8:dir 9:result.txt
sub()
{
    mkdir -p "$8/log"
    local logFilename="$8/log/log_f-$(printf "%.2e" $6)_seed-$7.log"
    local inputFilename="$8/log/imput_f-$(printf "%.2e" $6)_seed-$7.txt"
    local errorFilename="$8/log/err_f-$(printf "%.2e" $6)_seed-$7.err"

    touch "$logFilename" "$inputFilename" "$errorFilename"

    echo "$2 $3 $4 $5 $6 $7 $8" > "$inputFilename"

    # 检测调度系统
    if command -v sbatch >/dev/null 2>&1; then
        sbatch --job-name="$3-$4-$5-$6-$7" --output="$logFilename" --error="$errorFilename" --wrap="$1 < $inputFilename" >> "$8/log/$9"
    elif command -v bsub >/dev/null 2>&1; then
        bsub -n 1 -q normal -J "$3-$4-$5-$6-$7" -i "$inputFilename" -o "$logFilename" -e "$errorFilename" $1 >> "$8/log/$9"
    else
        echo "Unsupported scheduling system."
        return 1  # 可能需要根据实际情况考虑是否退出
    fi

    echo " sub $2_$3_$4_$5_$6_$7 "
}


#导入配置模块

config_file="config.sh"

# Check if the configuration file exists
 if [ ! -f "$config_file" ]; then
   echo "Configuration file does not exist!"
     exit 1
  fi
source $config_file

# 用于提醒
grep -v '^#' $config_file

while true; do
    read -p "Are you sure? (y/n) > " flag
    case "$flag" in
        y|Y) break ;;
        n|N) exit 1 ;;
        *) echo "Please answer y or n." ;;
    esac
done

# 写入日志
current_datetime=$(date +"%Y-%m-%d %H:%M:%S")
echo "$current_datetime" >> "log_sub"

grep -v '^#' $config_file >> "log_sub"
echo -e "\n\n" >> "log_sub"



# 定义主程序和数据目录路径
script_dir=$(dirname "$(realpath "$0")") # 获取脚本所在目录的绝对路径
project_dir=$(realpath "$script_dir/..") # 转到当前项目目录
jobpath="$project_dir/maincode/job" # 获取主程序路径
data_dir="$project_dir/data"

for N in "${N_array[@]}"; do
    for T in "${T_array[@]}"; do
        for t_p in "${t_p_array[@]}"; do
            for phi in "${phi_array[@]}"; do
                # 构建最终目录并直接创建，无需检查存在性
                dir="${data_dir}/T=${T}/t_p=${t_p}/phi-${phi}"
                mkdir -p "${dir}"

                for f in "${f_array[@]}"; do
                    for seed in "${seed_array[@]}"; do
                        # 参数位置 1:jobpath 2:N 3:T 4:t_p 5:phi 6:f 7: seed 8:dir 9:result.txt
                        sub "$jobpath" "$N" "$T" "$t_p" "$phi" "$f" "$seed" "$dir" "result.txt"
                    done
                done
            done
        done
    done
done