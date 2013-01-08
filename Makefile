

all compile build:
	rebar compile


# Need to build to make sure the nif module is built
test:
	rebar compile
	rebar eunit


doc:
	rebar doc


clean:
	rebar clean
	rm -rf doc



.PHONY: all compile build test doc clean
