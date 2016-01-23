-module(ert_server).
-compile(export_all).

data_handler(ProcWriting, _Data) ->
    ProcWriting ! { stream, <<"HTTP/1.1 200 OK\r\n"
        "Content-Length: 5\r\n\r\nHello">> }.

start_writing(Socket) ->
    receive
        { stream, Data } ->
            gen_tcp:send(Socket, Data),
            start_writing(Socket)
    end.

reading_loop(Socket, State, BufList, Handler)->
    { NetworkStatus, CombBufList } =
    %case gen_tcp:recv(Socket, 0) of
    receive
        { ok, Packet } -> 
            io:format(Packet),
            { ok, [Packet|BufList] };
        { error, Reason } ->
            error_logger:warning_msg("<Reading> ~p~n", [Reason]),
            gen_tcp:close(Socket),
            { bad, BufList }
    end,

    { DecodedState, DecodedBufList } =
    case ert_coder:decode(State, CombBufList) of
        { ok, Data, NewBufList } ->
            Handler(Data),
            { ert_coder:new_decode_state(), NewBufList };
        { partial, NewState } ->
            { NewState, [] };
    end,
    io:format("~p~n", [{ DecodedState, DecodedBufList }]),

    case NetworkStatus of
        ok -> reading_loop(Socket, DecodedState, DecodedBufList, Handler);
        bad -> iolist_to_binary(DecodedBufList)
    end.

start_reading(Socket, Handler) ->
    reading_loop(Socket, ert_coder:new_decode_state(), [], Handler).

acceptor(ListenSocket) ->
    case gen_tcp:accept(ListenSocket) of
        { ok, Socket } ->
            ProcWriting = spawn(fun() -> start_writing(Socket) end),
            ProcReading = spawn(fun() -> start_reading(Socket,
                fun(Data) -> data_handler(ProcWriting, Data) end) end)
    end,
    acceptor(ListenSocket).

listener(Port) ->
    case gen_tcp:listen(Port, [{active, true}]) of
        { ok, ListenSocket } ->
            acceptor(ListenSocket);
        { error, Reason } ->
            error_logger:warning_msg("<Listener> ~p~n", [Reason])
    end.

