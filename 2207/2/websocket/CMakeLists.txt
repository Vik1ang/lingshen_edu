add_executable(websocket_c websocket.c)

set(DEP_LIBS OpenSSL::Crypto OpenSSL::SSL)

target_link_libraries(websocket_c ${DEP_LIBS})