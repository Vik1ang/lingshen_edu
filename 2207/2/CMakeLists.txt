add_subdirectory(2.1.1_select_poll_epoll)
add_subdirectory(2.1.2_reactor)
add_subdirectory(2.1.3_http_https)

add_executable(select_c select.c)
target_link_libraries(select_c PUBLIC Threads::Threads)

add_executable(select_cpp select.cpp)
target_link_libraries(select_cpp PUBLIC Threads::Threads)

add_executable(epoll_c epoll.c)
#target_link_libraries(epoll_c PUBLIC Threads::Threads)

add_executable(epoll_cpp epoll.cpp)
#target_link_libraries(epoll_cpp PUBLIC Threads::Threads)

add_executable(reactor_cpp reactor.cpp)

add_executable(reactor_c reactor.c)

add_executable(mul_port_client_epoll mul_port_client_epoll.c)

add_executable(test test.c)