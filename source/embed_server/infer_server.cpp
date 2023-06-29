#include "infer_server.hpp"
#include <cassert>

using std::vector;
using grpc::Status;

InferServerImpl::InferServerImpl(int n_part, int rank,vector<std::string> ServerAddressList) : n_part_(n_part), rank_(rank){
    assert(n_part_ == ServerAddressList.size());
    for(int i = 0; i < n_part_; i++){
        auto channel = grpc::CreateChannel(ServerAddressList[i], grpc::InsecureChannelCredentials());
        stub_list_.emplace_back(EmbedServer::NewStub(channel));
    }
}

Status InferServerImpl::Inference(grpc::ServerContext* context, const InferenceRequest* request, InferenceReply* reply){
    if(request->data_size() != request->pos_size()) return Status::OK;
    int n = request -> data_size();
    vector<EmbedRequest> request_list(n_part_);
    vector<AsyncClientCall> call_list(n_part_);
    EmbedReply embed_reply;
    for(int i = 0; i < n; i++){
        // std::cout << i << " " << request->pos(i) << " " << request->data(i) << std::endl;
        request_list[request->pos(i)].add_data(request->data(i));
    }
    int all_calls = 0;
    for(int i = 0; i < n_part_; i++){
        if(i == rank_) continue;
        if(request_list[i].data_size() > 0){
            call_list[i].response_reader = stub_list_[i]->PrepareAsyncLookup(&call_list[i].context, request_list[i], &cq_);
            call_list[i].response_reader->StartCall();
            call_list[i].response_reader->Finish(&call_list[i].reply, &call_list[i].status, (void*)&call_list[i]);
            all_calls++;
        }
    }
    vector<vector<float>> results;
    int completed_calls = 0;
    while (completed_calls < all_calls){
        void* got_tag;
        bool ok = false;
        if (cq_.Next(&got_tag, &ok)) {
            GPR_ASSERT(ok);
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);
            if (call->status.ok()) {
                for (int i = 0; i < call->reply.embed_values_size(); ++i) {
                    SingleEmbed se = call->reply.embed_values(i);
                    vector<float> tmp(se.data_size());
                    for(int j = 0; j < se.data_size(); ++j) tmp[j] = se.data(j);
                    results.push_back(tmp);
                }
            } else {
                std::cout << "RPC failed" << std::endl;
                abort();
            }
            ++completed_calls;
        }
    }
    for(int i = 0; i < n; i++){
        reply->add_data(request->data(i));
    }
    return Status::OK;
}

InferServerSyncImpl::InferServerSyncImpl(int n_part, int rank,vector<std::string> ServerAddressList) : n_part_(n_part), rank_(rank){
    assert(n_part_ == ServerAddressList.size());
    for(int i = 0; i < n_part_; i++){
        auto channel = grpc::CreateChannel(ServerAddressList[i], grpc::InsecureChannelCredentials());
        stub_list_.emplace_back(EmbedServer::NewStub(channel));
    }
}

Status InferServerSyncImpl::Inference(grpc::ServerContext* context, const InferenceRequest* request, InferenceReply* reply){
    if(request->data_size() != request->pos_size()) return Status::OK;
    int n = request -> data_size();
    vector<EmbedRequest> request_list(n_part_);
    vector<EmbedReply> reply_list(n_part_);
    grpc::ClientContext client_context;
    for(int i = 0; i < n_part_; i++){
        if(i == rank_) continue;
        if(request_list[i].data_size() > 0){
            stub_list_[i] -> Lookup(&client_context, request_list[i], &reply_list[i]);
        }
    }
    for(int i = 0; i < n; i++){
        reply->add_data(request->data(i));
    }
    return Status::OK;
}