#!/bin/bash
SCRIPT_DIR="$(dirname "$0")"
cd "$SCRIPT_DIR/.."

# 定义服务器地址列表
servers=("49.52.27.23" "49.52.27.25" "49.52.27.26" "49.52.27.32")

operation=$1
if [ "$operation" != "sync" ] && [ "$operation" != "start" ] && [ "$operation" != "stop" ]; then
    echo "Invalid operation. Please specify 'sync', 'start', or 'stop'."
    exit 1
fi

# 循环每个服务器
for server in "${servers[@]}"
do
    echo $server
    # 根据操作类型执行相应的命令
    if [ "$operation" == "sync" ]; then
        # 拷贝可执行文件到远程服务器
        scp build/bin/embed_server $server:~
    elif [ "$operation" == "start" ]; then
        # 在远程服务器上启动程序
        ssh -n $server 'chmod +x ~/embed_server && nohup ~/embed_server > output.log 2>&1 &'
        if [ $? -ne 0 ]; then
            echo "Failed to start embed_server on $server"
            exit 1
        fi
    elif [ "$operation" == "stop" ]; then
        # 在远程服务器上停止程序
        ssh $server 'pkill embed_server'
    fi
done