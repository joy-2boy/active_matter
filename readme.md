# 项目说明
## active_md_lib
- cell_md 使用元胞法实现分子动力学模拟
- genfilename 用来生成数据文件名
- init 初始化体系
- mathe 在标准的math库上增加了一个额外的随机数产生器，不喜欢可以方便的更换（保持函数名一致）
- statetracker 各种表征系统的函数
- sys 系统信息
## script/bulid.sh
构建项目的脚本，会在 源代码目录下生成一个名为 job 的可执行文件
```
./script/build.sh ${filename} [-g] [-p]
```
- -g表示生调试信息
- -p表示生成生成 gprof 的的分析文件

## .vscode
vscode 配置文件

## main_dev
- main.c 项目主代码
- data 存放数据
- sub 提交任务脚本和提交日志 



