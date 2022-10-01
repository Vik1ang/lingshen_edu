add_executable(server server.cpp)
target_link_libraries(server PUBLIC Threads::Threads)
#add_executable(tcp_server tcp_server.cpp)