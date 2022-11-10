add_executable(websocket_c websocket.c)

find_package(OpenSSL)

if (OpenSSL_FOUND)
    set(DEP_LIBS OpenSSL::Crypto OpenSSL::SSL)
    target_link_libraries(websocket_c ${DEP_LIBS})
else()
    message(WARNING "Could not find OpenSSL")
endif()
