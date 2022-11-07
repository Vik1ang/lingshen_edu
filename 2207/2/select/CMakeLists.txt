add_executable(select_c select.c)
target_link_libraries(select_c PUBLIC Threads::Threads)

add_executable(select_cpp select.cpp)
target_link_libraries(select_cpp PUBLIC Threads::Threads)