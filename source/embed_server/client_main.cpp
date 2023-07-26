#include "infer_client.hpp"
#include "../utils/utils.hpp"
#include <iostream>
#include <chrono>
#include "CLI11.hpp"
#include "../merge/merger.hpp"

using std::vector;

const static std::string CRITEO_PATH = "/home/xuzhizhen/datasets/criteo-tb/";
constexpr static int sample_factor = 10;
constexpr static int batch_size = 1;
constexpr static int num_threads = 32;
constexpr static double hot_rate = 0.0005;
constexpr static bool is_stat_only = false;
const static vector<std::string> server_address_list = {"49.52.27.23:50051","49.52.27.25:50051","49.52.27.26:50051","49.52.27.27:50051"};

void simpleTest(){
    Dispatcher dispatcher(4, server_address_list);
    dispatcher.LoadPartitionNPZ(CRITEO_PATH + "partition/window_10/day0_20m.npz",hot_rate);
    vector<int> input1 = {81025217,  92483388,  92497334,  92506222,  92513787,  92529243,
        92530645,  92535997,  92537134,  94640450, 104267943, 104988298,
       105079864, 105081565, 105087455, 105091984, 105092072, 105092795,
       105093113, 121914269, 127475318, 133851089, 147705075, 147756120,
       147761672, 147761816};
    vector<int> input2 = {90567205,  92475843,  92493516,  92508218,  92514122,  92529243,
        92533624,  92536119,  92537125,  98292522, 104486409, 104991959,
       105079864, 105081565, 105091459, 105091984, 105092072, 105092704,
       105093113, 127007672, 129908815, 144023219, 147486485, 147755808,
       147761715, 147761809};
    for(int i = 0; i < 1; i++){
        dispatcher.DispatchRequest(input1);
    }
}

void runExperiment(){
    const std::string filename = "results/latency.csv";
    std::ofstream outfile(filename.c_str());
    if(!outfile.is_open()){
        outfile.open(filename.c_str(), std::ios::out);
        if (!outfile) {
            std::cout << "Error: failed to create file " << filename << std::endl;
            return ;
        }
    }
    outfile <<"bias,p95,avg,data,node,p95,avg,data,node" << std::endl;
    Dispatcher dispatcher(4, server_address_list,is_stat_only);
    Dispatcher dispatcher1(4, server_address_list,is_stat_only);
    std::cout << "created dispatcher" << std::endl;
    dispatcher.LoadPartitionNPZ(CRITEO_PATH + "partition/window_10/day0_20m.npz",hot_rate);
    dispatcher1.LoadPartitionNPZ(CRITEO_PATH + "partition/window_10/day0_20m.npz",hot_rate);
    vector<vector<int>> data;
    load_data(CRITEO_PATH + "sparse_day_0.npy", data);
    Merger merger(4,1.05);
    // for(int i = 0; i < 10000; i++){
    //     auto currentInput = getCurrentInput(data,1,26,i);
    //     dispatcher.DispatchRequest(currentInput);
    //     dispatcher1.DispatchRequest(currentInput);
    // }
    dispatcher.clearTime();
    dispatcher1.clearTime();
    std::cout << "test start" << std::endl;
    // for(int k = 10; k < 20; k++){
    //     merger.update(CRITEO_PATH + "partition/new_10/day0_" + std::to_string(k) + "m.bin");
    // }
    for(int k = 20; k <= 180; k++){
        merger.update(CRITEO_PATH + "partition/new_window_1/day0_" + std::to_string(k) + "m.bin");
        dispatcher.LoadPartitionMerge(merger.generatePartition(hot_rate));
        // dispatcher1.LoadPartitionNPZ(CRITEO_PATH + "partition/window_10/day0_"+ std::to_string(k) +"m.npz",hot_rate);
        // int bias = k * 1000000;
        int bias = k * 1000000;
        auto start = std::chrono::high_resolution_clock::now();
        #pragma omp parallel for num_threads(num_threads)
        for(int i = 0; i < 1000000; i+=sample_factor * batch_size){
            auto currentInput = getCurrentInput(data, batch_size, 26, bias + i);
            dispatcher1.DispatchRequest(currentInput);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double t1 = elapsed_seconds.count();
        double qps1 = (1000000/sample_factor)/t1;
        start = std::chrono::high_resolution_clock::now();
        #pragma omp parallel for num_threads(num_threads)
        for(int i = 0; i < 1000000; i+=sample_factor * batch_size){
            auto currentInput = getCurrentInput(data, batch_size, 26, bias + i);
            dispatcher.DispatchRequest(currentInput);
        }
        end = std::chrono::high_resolution_clock::now();
        elapsed_seconds = end - start;
        double t0 = elapsed_seconds.count();
        double qps0 = (1000000/sample_factor)/t0;
        std::cout << bias;
        std::cout << " " << dispatcher.getTailTime()<< " " << dispatcher.getAvgTime() << " " << dispatcher.getRemoteCount() << " " << dispatcher.getNodeCount() << " " << qps0;
        std::cout << " " << dispatcher1.getTailTime() << " " << dispatcher1.getAvgTime() << " " << dispatcher1.getRemoteCount() << " " << dispatcher1.getNodeCount() << " " << qps1;
        std::cout << std::endl;
        outfile << bias;
        outfile << "," << dispatcher.getTailTime()<< "," << dispatcher.getAvgTime() << "," << dispatcher.getRemoteCount() << "," << dispatcher.getNodeCount();
        outfile << "," << dispatcher1.getTailTime() << "," << dispatcher1.getAvgTime() << "," << dispatcher1.getRemoteCount() << "," << dispatcher1.getNodeCount();
        outfile << std::endl;
        // dispatcher.getDebugInfo();
        // dispatcher1.getDebugInfo();
        dispatcher.clearTime();
        dispatcher1.clearTime();
    }
    std::cout << "all request finished " <<std::endl; 
}

auto main()->int{
    // simpleTest();
    runExperiment();
    return 0;
}