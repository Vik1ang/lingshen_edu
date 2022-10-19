add_executable(epoll_c epoll_c.c)
target_link_libraries(epoll_c PUBLIC Threads::Threads)

add_executable(epoll_cpp epoll_cpp.cpp)
target_link_libraries(epoll_cpp PUBLIC Threads::Threads)

add_executable(mul_port_client_epoll mul_port_client_epoll.c)
target_link_libraries(mul_port_client_epoll PUBLIC Threads::Threads)

add_executable(reactor_c_1 reactor_c_1.c)
target_link_libraries(reactor_c_1 PUBLIC Threads::Threads)

add_executable(reactor_c_2 reactor_c_2.c)
target_link_libraries(reactor_c_2 PUBLIC Threads::Threads)