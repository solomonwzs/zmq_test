add_executable(client client.c)
add_executable(server server.c)

target_link_libraries(client pthread zmq)
target_link_libraries(server pthread zmq base)
