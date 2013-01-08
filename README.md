
XML Schema Definition Regular Expressions, xsdre
================================================

This Erlang library implements [W3C XML Schema regular expression]
(http://www.w3.org/TR/2012/REC-xmlschema11-2-20120405/#regexs)
functionality. It is implemented with NIF:s that use the regular
expression functions in [libxml2]
(http://www.xmlsoft.org/html/libxml-xmlregexp.html).

For a good summary of the difference between different regular
expression implementations see,
[http://www.regular-expressions.info/refflavors.html].


Copyright
---------

This library is copyrighted by Tail-f Systems, you are permitted to
use it in accordance with the licence in the file LICENCE.


Compiling
---------

The library is built using [rebar](https://github.com/basho/rebar) (of
which you will need a recent version, 2.1.x). A Makefile is provided
as convenience (but it only invokes rebar).


Documentation
-------------

Run "make doc" to build the edoc documentation.

