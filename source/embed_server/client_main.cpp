#include "infer_client.hpp"
#include "../utils/utils.hpp"
#include <iostream>

using std::vector;

const static std::string CRITEO_PATH = "/home/xuzhizhen/datasets/criteo-tb/";
const static int sample_factor = 1000;

auto main()->int{
    std::string server_address("0.0.0.0:50051");
    vector<std::string> server_address_list= {"49.52.27.23:50051","49.52.27.25:50051","49.52.27.26:50051","49.52.27.32:50051"};
    Dispatcher dispatcher(4, server_address_list);
    std::cout << "created dispatcher" << std::endl;
    dispatcher.LoadPartitionNPZ(CRITEO_PATH + "partition/merged/day0_80m.npz",0.001);

    vector<vector<int>> data;
    load_data(CRITEO_PATH + "sparse_day_0.npy", data);
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
    dispatcher.DispatchRequest(input1);
    dispatcher.clearTime();
    for(int i = 20000000; i <= 80000000; i+=sample_factor){
        auto currentInput = getCurrentInput(data,1,26,i);
        dispatcher.DispatchRequest(currentInput);
        if(i % 1000000 == 0){
            std::cout << i << " " << dispatcher.getAvgTime() << std::endl;
            dispatcher.clearTime();
            // dispatcher.LoadPartitionNPZ(CRITEO_PATH + "partition/full_5/day0_"+ std::to_string(i/1000000) +"m.npz",0.001);
        }
    }
    std::cout << "all request finished " <<std::endl; 
    return 0;
}