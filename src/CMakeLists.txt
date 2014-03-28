find_path(ZEROMQ_INCLUDE_DIR zmq.h
    /usr/include
    /usr/local/zeromq)

find_path(ZEROMQ_LIB_DIR libzmq.so
    /usr/lib
    /usr/local/zeromq)

link_directories(${ZEROMQ_LIB_DIR} /usr/lib)

add_library(base STATIC base.c)

add_subdirectory(erl)
add_subdirectory(helloworld)
