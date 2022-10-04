add_executable(server server.cpp)
target_link_libraries(server PUBLIC Threads::Threads)

add_executable(tcp_server_cstyle tcp_server_cstyle.c)
target_link_libraries(tcp_server_cstyle PUBLIC Threads::Threads)
#add_executable(tcp_server tcp_server.cpp)
