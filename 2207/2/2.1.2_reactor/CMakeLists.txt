add_executable(epoll_c epoll_c.c)
target_link_libraries(epoll_c PUBLIC Threads::Threads)

add_executable(epoll_cpp epoll_cpp.cpp)
target_link_libraries(epoll_cpp PUBLIC Threads::Threads)

add_executable(reactor_c reactor_c.c)
target_link_libraries(reactor_c PUBLIC Threads::Threads)