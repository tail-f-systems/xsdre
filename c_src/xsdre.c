/*
 * NIF implementation of the xsdre module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/xmlregexp.h>

#include "erl_nif.h"


/* ---------------------------------------------------------------------- */

struct regexp {
    xmlRegexpPtr xre;
    char *string;
};

static ErlNifResourceType *regexp_type = NULL;

/* ---------------------------------------------------------------------- */

/* Commonly used atoms, initialized at load time */
static ERL_NIF_TERM am_true;
static ERL_NIF_TERM am_false;
static ERL_NIF_TERM am_ok;
static ERL_NIF_TERM am_error;
static ERL_NIF_TERM am_match;
static ERL_NIF_TERM am_nomatch;

static void initialize_local_atoms(ErlNifEnv* env)
{
    am_true    = enif_make_atom(env, "true");
    am_false   = enif_make_atom(env, "false");
    am_ok      = enif_make_atom(env, "ok");
    am_error   = enif_make_atom(env, "error");
    am_match   = enif_make_atom(env, "match");
    am_nomatch = enif_make_atom(env, "nomatch");
}

/* Not strictly necessary for atoms, but future proof.. */
#define am_true(ENV)    enif_make_copy(ENV, am_true)
#define am_false(ENV)   enif_make_copy(ENV, am_false)
#define am_ok(ENV)      enif_make_copy(ENV, am_ok)
#define am_error(ENV)   enif_make_copy(ENV, am_error)
#define am_match(ENV)   enif_make_copy(ENV, am_match)
#define am_nomatch(ENV) enif_make_copy(ENV, am_nomatch)



/* ---------------------------------------------------------------------- */

static ERL_NIF_TERM last_error(ErlNifEnv *env, char *fallback)
{
    char *default_msg = (fallback) ? fallback : "Unknown";
    /* This is thread safe as long as libxml2 has been compiled with
     * thread support (alternatively use a mutex) */
    xmlErrorPtr err = xmlGetLastError();

    /* Seems the useful error is in str1? */
    char *err_str =
        (err && err->str1) ? err->str1 :
        (err && err->message) ? err->message : default_msg;

    return
        enif_make_tuple2(env,
                         am_error(env),
                         enif_make_string(env, err_str, ERL_NIF_LATIN1));
}


static ERL_NIF_TERM compile(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary patternbin;

    if (enif_inspect_iolist_as_binary(env, argv[0], &patternbin)) {
        ERL_NIF_TERM ret;
        xmlRegexpPtr xre;
        char *patternstr = enif_alloc(patternbin.size + 1);
        patternstr[patternbin.size] = 0;
        memcpy(patternstr, patternbin.data, patternbin.size);

        if ((xre = xmlRegexpCompile((xmlChar *)patternstr)) != NULL) {
            struct regexp *r;
            r = enif_alloc_resource(regexp_type, sizeof(struct regexp));
            memset(r, 0, sizeof(*r));
            r->xre = xre;
            r->string = patternstr;

            /* transfer ownership to calling process */
            ret = enif_make_tuple2(env, am_ok(env),
                                   enif_make_resource(env, r));
            enif_release_resource(r);
        } else {
            enif_free(patternstr);
            ret = last_error(env, "Bad Pattern");
        }

        return ret;
    }

    return enif_make_badarg(env);
}

static ERL_NIF_TERM match(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    struct regexp *rp = NULL;
    struct regexp r;
    ErlNifBinary strbin;
    ERL_NIF_TERM ret;

    if (!enif_get_resource(env, argv[1], regexp_type, (void **)&rp)) {
        ErlNifBinary patternbin;
        if (enif_inspect_iolist_as_binary(env, argv[1], &patternbin)) {
            char patternstr[patternbin.size + 1];
            xmlRegexpPtr xre;
            patternstr[patternbin.size] = 0;
            memcpy(patternstr, patternbin.data, patternbin.size);
            if ((xre = xmlRegexpCompile((xmlChar *)patternstr)) != NULL) {
                r.xre = xre;
                r.string = NULL;
                rp = &r;
            } else {
                return last_error(env, "Bad Pattern");
            }
        } else {
            return enif_make_badarg(env);
        }
    }

    if (enif_inspect_iolist_as_binary(env, argv[0], &strbin)) {
        char string[strbin.size + 1];
        string[strbin.size] = 0;
        memcpy(string, strbin.data, strbin.size);
        switch (xmlRegexpExec(rp->xre, (xmlChar *)string)) {
        case 1:
            ret = am_true(env);
            break;
        case 0:
            ret = am_false(env);
            break;
        default:
            ret = last_error(env, NULL);
        }
    } else {
        ret = enif_make_badarg(env);
    }

    if (rp == &r) {
        xmlRegFreeRegexp(r.xre);
    }

    return ret;
}

static ERL_NIF_TERM run(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    struct regexp *rp = NULL;
    struct regexp r;
    ErlNifBinary strbin;
    ERL_NIF_TERM ret;

    if (!enif_get_resource(env, argv[1], regexp_type, (void **)&rp)) {
        ErlNifBinary patternbin;
        if (enif_inspect_iolist_as_binary(env, argv[1], &patternbin)) {
            char patternstr[patternbin.size + 1];
            xmlRegexpPtr xre;
            patternstr[patternbin.size] = 0;
            memcpy(patternstr, patternbin.data, patternbin.size);
            if ((xre = xmlRegexpCompile((xmlChar *)patternstr)) != NULL) {
                r.xre = xre;
                r.string = NULL;
                rp = &r;
            } else {
                return last_error(env, "Bad Pattern");
            }
        } else {
            return enif_make_badarg(env);
        }
    }

    if (enif_inspect_iolist_as_binary(env, argv[0], &strbin)) {
        char string[strbin.size + 1];
        string[strbin.size] = 0;
        memcpy(string, strbin.data, strbin.size);
        switch (xmlRegexpExec(rp->xre, (xmlChar *)string)) {
        case 1:
            /* FIXME NYI */
            ret = enif_make_tuple2(env, am_match(env),
                                   enif_make_list(env, 0));
            break;
        case 0:
            ret = am_nomatch(env);
            break;
        default:
            ret = last_error(env, NULL);
        }
    } else {
        ret = enif_make_badarg(env);
    }

    if (rp == &r) {
        xmlRegFreeRegexp(r.xre);
    }

    return ret;
}

static ERL_NIF_TERM string(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    struct regexp *r = NULL;
    if (!enif_get_resource(env, argv[0], regexp_type, (void **)&r)) {
        return enif_make_badarg(env);
    }
    return enif_make_string(env, r->string, ERL_NIF_LATIN1);
}

static ERL_NIF_TERM is_xre(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    struct regexp *r = NULL;
    if (enif_get_resource(env, argv[0], regexp_type, (void **)&r)) {
        return am_true(env);
    }
    return am_false(env);
}

static ERL_NIF_TERM is_deterministic(ErlNifEnv *env, int argc,
                                     const ERL_NIF_TERM argv[])
{
    struct regexp *r = NULL;
    if (!enif_get_resource(env, argv[0], regexp_type, (void **)&r)) {
        return enif_make_badarg(env);
    }
    switch (xmlRegexpIsDeterminist(r->xre)) {
    case 0:
        return am_false(env);
    case 1:
        return am_true(env);
    default:
        return last_error(env, NULL);
    }
}


static ErlNifFunc nif_funcs[] = {
    {"compile", 1, compile},
    {"match", 2, match},
    {"run", 2, run},
    {"string", 1, string},
    {"is_xre", 1, is_xre},
    {"is_deterministic", 1, is_deterministic}
};


static void destroy_regexp(ErlNifEnv *env, void *obj)
{
    struct regexp *r = obj;
    xmlRegFreeRegexp(r->xre);
    enif_free(r->string);
}

/* The default error handle will print to stderr */
static void libxml_error_handler(void *ctx, const char *msg, ...)
{
    return;
}


static int atload(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    ErlNifResourceType *rt;

    initialize_local_atoms(env);

    xmlSetGenericErrorFunc(NULL, libxml_error_handler);

    rt = enif_open_resource_type(env, "xsdre", "regexp", destroy_regexp,
                                 ERL_NIF_RT_CREATE, NULL);

    if (rt == NULL) return -1;

    regexp_type = rt;

    return 0;
}

ERL_NIF_INIT(xsdre, nif_funcs, atload, NULL, NULL, NULL)
