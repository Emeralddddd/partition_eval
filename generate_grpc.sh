./build/bin/protoc -I source/proto --grpc_out=build/source/embed_server --plugin=protoc-gen-grpc=build/bin/grpc_cpp_plugin embed_server.proto inference_request.proto
./build/bin/protoc -I source/proto --cpp_out=build/source/embed_server embed_server.proto inference_request.proto
protoc -I source/proto --cpp_out=build/source/merge partial_result.proto