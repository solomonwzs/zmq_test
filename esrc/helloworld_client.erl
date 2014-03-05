-module(helloworld_client).

-export([main/0, send/1]).


main()->
    {ok, Context}=erlzmq:context(),

    io:format("Connecting to hello world server...~n"),
    {ok, Requester}=erlzmq:socket(Context, req),
    ok=erlzmq:connect(Requester, "tcp://localhost:5555"),

    lists:foreach(
     fun(N)->
             io:format("Sending Hello ~b....~n", [N]),
             ok=erlzmq:send(Requester, term_to_binary(100.1)),

             {ok, Reply}=erlzmq:recv(Requester),
             io:format("Received ~p ~b~n", [Reply, N])
     end, lists:seq(1, 10)),

    ok=erlzmq:close(Requester),
    ok=erlzmq:term(Context).


send(Term)->
    {ok, Context}=erlzmq:context(),
    {ok, Requester}=erlzmq:socket(Context, push),
    ok=erlzmq:connect(Requester, "tcp://192.168.1.50:5555"),

    ok=erlzmq:send(Requester, term_to_binary(Term)),
    io:format("send: ~p~n", [Term]),

    %{ok, Reply}=erlzmq:recv(Requester),
    %io:format("received: ~p~n", [binary_to_term(Reply)]),

    ok=erlzmq:close(Requester),
    ok=erlzmq:term(Context).
