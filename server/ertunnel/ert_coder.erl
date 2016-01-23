-module(ert_coder).
-compile(export_all).

-record(decode_state, { read_lines = [], cur_line = "", stream = <<>> }).

parse_header_line(ReadLines, CurLine, <<"\r\n", DecBinary/binary>>) ->
    CompleteLine = lists:reverse([$\n, $\r|CurLine]),
    case CompleteLine of
        "\r\n" ->
            { ok, lists:reverse([CompleteLine|ReadLines]), DecBinary };
        _Str ->
            parse_header_line([CompleteLine|ReadLines], [], DecBinary)
    end;

parse_header_line(ReadLines, CurLine, <<>>) ->
    { partial, CurLine, ReadLines, <<>> };

parse_header_line(ReadLines, CurLine, <<Char:8, DecBinary/binary>>) ->
    parse_header_line(ReadLines, [Char|CurLine], DecBinary).

decode({ state, { OldLines, LastLine, OldBinary }}, BufList) ->
    Binary = iolist_to_binary([OldBinary|BufList]),
    case parse_header_line(OldLines, LastLine, Binary) of
        { ok, Lines, DecBinary } ->
            { ok, Lines, [DecBinary] };
        { partial, PartialLine, ReadLines, DecBinary } ->
            { partial, { state, { ReadLines, PartialLine, DecBinary }} }
    end.

new_decode_state() -> { state, #decode_state{} }.


