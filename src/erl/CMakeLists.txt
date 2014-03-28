find_path(ERL_INCLUDE_DIR ei.h
    /usr/lib/erlang/usr/include/)

find_path(ERL_LIB_DIR libei.a
    /usr/lib/erlang/usr/lib/)

link_directories(${ERL_LIB_DIR})
include_directories(${ERL_INCLUDE_DIR})

add_executable(server1 server1.c)
add_executable(server2 server2.c)

target_link_libraries(server1 pthread zmq erl_interface ei base)
target_link_libraries(server2 pthread zmq erl_interface ei base)
