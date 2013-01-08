%%% ------------------------------------------------------------------------
%%% @copyright 2011,2012,2013 Tail-f Systems AB
%%% @doc XML Schema regular expression library.
%%%
%%% This library implements
%%% <a href="http://www.w3.org/TR/2012/REC-xmlschema11-2-20120405/#regexs">
%%% W3C XML Schema regular expressions </a>, using NIF:s towards the
%%% regular expression functions in
%%% <a href="http://www.xmlsoft.org/html/libxml-xmlregexp.html">libxml2</a>.
%%% For a good summary of the difference between different regular
%%% expression implementation see,
%%% [http://www.regular-expressions.info/refflavors.html].
%%%
%%% @end
%%% ------------------------------------------------------------------------
-module(xsdre).

-export([compile/1, run/2, match/2]).
-export([string/1, is_xre/1, is_deterministic/1]).

-on_load(on_load/0).

-opaque xre() :: <<>>.
-type binstring() :: iolist() | binary().

-export_type([xre/0]).


-spec compile(Pattern::binstring()) -> {'ok', xre()} | {'error', string()}.
%% @doc Compile a regular expression pattern. The Pattern is compiled
%% and, if successful, a resource that can be used in subsequent calls
%% to {@link match/2} or {@link run/2} is returned.
compile(_Pattern) ->
    nif_only().


-spec run(String::binstring(), RegExp::xre() | binstring()) ->
                 {'match', Captured::term()} | 'nomatch' |
                 {'error', string()}.
%% @doc Run RegExp on String.
%%
%% <em>Returning Captured is not yet implemented, currently it is
%% always [].</em>
%% 
%% Note that if you are going to use the same pattern many times it is more
%% efficient to compile it once and keep the compiled expression.
run(_String, _RegExp) ->
    nif_only().


-spec match(String::binstring(), RegExp::xre() | binstring()) ->
                   boolean() | {'error', string()}.
%% @doc Run RegExp on String and return true if it matches.
%%
%% This function is equivalent to {@link run/2} except it has a
%% boolean return value.
%%
%% Note that if you are going to use the same pattern many times it is more
%% efficient to compile it once and keep the compiled expression.
match(_String, _RegExp) ->
    nif_only().


-spec is_deterministic(xre()) -> boolean().
%% @doc Returns true if the compiled regular expression is deterministic.
is_deterministic(_CompiledRegex) ->
    nif_only().


-spec is_xre(term()) -> boolean().
%% @doc Returns true if term is a compiled regular expression (as
%% returned by {@link compile/1})
is_xre(_CompiledRegex) ->
    nif_only().


-spec string(xre()) -> string().
%% @doc Return the pattern (as a string) which this regexp corresponds to.
%% This is the original pattern given in compile/1.
string(_CompiledRegex) ->
    nif_only().


on_load() ->
    PrivDir =
        case code:priv_dir(?MODULE) of
            {error, bad_name} ->
                filename:join([filename:dirname(code:which(?MODULE)),
                               "..","priv"]);
            Path ->
                Path
        end,
    NifFile = filename:join(PrivDir, lists:concat([?MODULE, "_nif"])),
    erlang:load_nif(NifFile, 0).

nif_only() ->
    erlang:nif_error({nif_not_loaded, ?MODULE}).


%%%
%%% Unit Tests
%%%
-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").

xsdre_test_() ->
    [
     ?_test(true = match("abc", "[a-c]+"))
     , ?_test({match,_} = run("abc", "[a-c]+"))

     , ?_test(begin {ok, X} = compile("[a-c]+"), true = match("abc", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), true = match("cc", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), false = match("", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), false = match("x", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), false = match("xyz", X) end)
     
     , ?_test(begin {ok, X} = compile("[a-c]+"), {match, _} = run("abc", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), {match, _} = run("cc", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), nomatch = run("", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), nomatch = run("x", X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), nomatch = run("xyz", X) end)

     , ?_test(begin {ok, X} = compile("[a-c]+"), true = is_xre(X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), true = is_deterministic(X) end)
     , ?_test(begin {ok, X} = compile("[a-c]+"), "[a-c]+" = string(X) end)
     
     , ?_test({error, _Str} = compile("[a-c"))
    ].

-endif.
