add_subdirectory(2.1.1_select_poll_epoll)
add_subdirectory(2.1.2_reactor)
add_subdirectory(2.1.3_http_https)

add_executable(select_c select.c)
target_link_libraries(select_c PUBLIC Threads::Threads)

add_executable(select_cpp select.cpp)
target_link_libraries(select_cpp PUBLIC Threads::Threads)