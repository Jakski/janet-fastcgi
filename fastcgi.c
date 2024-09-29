#define _GNU_SOURCE
#include <janet.h>
#include <string.h>
#include <fcgiapp.h>

static FCGX_Stream *in, *out, *err;
static FCGX_ParamArray envp;

static void handle_error(FCGX_Stream *stream) {
	int ecode;
	char ebuf[256];
	ecode = FCGX_GetError(stream);
	switch (ecode) {
		case FCGX_UNSUPPORTED_VERSION:
			janet_panic("FCGX_UNSUPPORTED_VERSION");
			break;
		case FCGX_PROTOCOL_ERROR:
			janet_panic("FCGX_PROTOCOL_ERROR");
			break;
		case FCGX_PARAMS_ERROR:
			janet_panic("FCGX_PARAMS_ERROR");
			break;
		case FCGX_CALL_SEQ_ERROR:
			janet_panic("FCGX_CALL_SEQ_ERROR");
			break;
		default:
			if (ecode <= 0) {
				janet_panic("FCGX_Accept failed with unknown error");
			}
			janet_panic(strerror_r(ecode, ebuf, 256));
	}
}

static Janet janet_fastcgi_accept(int32_t argc, Janet *argv) {
	janet_fixarity(argc, 0);
	if (FCGX_Accept(&in, &out, &err, &envp) != 0) {
		handle_error(in);
	}
	return janet_wrap_nil();
}

static Janet janet_fastcgi_print_out(int32_t argc, Janet *argv) {
	const char *text;
	janet_fixarity(argc, 1);
	text = janet_getcstring(argv, 0);
	if (FCGX_FPrintF(out, "%s", text) == EOF) {
		handle_error(out);
	}
	return janet_wrap_nil();
}

static Janet janet_fastcgi_print_err(int32_t argc, Janet *argv) {
	const char *arg;
	janet_fixarity(argc, 1);
	arg = janet_getcstring(argv, 0);
	if (FCGX_FPrintF(err, "%s", arg) == EOF) {
		handle_error(out);
	}
	return janet_wrap_nil();
}

static Janet janet_fastcgi_get_param(int32_t argc, Janet *argv) {
	const char *name;
	char *value;
	janet_fixarity(argc, 1);
	name = janet_getcstring(argv, 0);
	value = FCGX_GetParam(name, envp);
	if (value == NULL) {
		return janet_wrap_nil();
	}
	return janet_cstringv(value);
}

static Janet janet_fastcgi_read(int32_t argc, Janet *argv) {
	int rlen, blen;
	uint8_t *buf;
	Janet r;
	janet_fixarity(argc, 1);
	blen = janet_getinteger(argv, 0);
	buf = janet_smalloc(blen);
	rlen = FCGX_GetStr((char *) buf, blen, in);
	r = janet_wrap_string(janet_string(buf, rlen));
	janet_sfree(buf);
	return r;
}

static Janet janet_fastcgi_read_all(int32_t argc, Janet *argv) {
	janet_fixarity(argc, 0);
	uint8_t *buf = NULL;
	uint8_t *cursor;
	int buflen = 0;
	int bodylen = 0;
	int readlen;
	do {
		buflen += 256;
		buf = janet_srealloc(buf, buflen);
		cursor = buf + (buflen - 256);
		readlen = FCGX_GetStr((char *) cursor, 256, in);
		bodylen += readlen;
	} while (readlen == 256);
	Janet r = janet_wrap_string(janet_string(buf, bodylen));
	janet_sfree(buf);
	return r;
}

static const JanetReg cfuns[] = {
	{
		"accept",
		janet_fastcgi_accept,
		"(fastcgi/accept)\n\nAccept FastCGI request.",
	},
	{
		"print-out",
		janet_fastcgi_print_out,
		"(fastcgi/print-out text)\n\nPrint text to output.",
	},
	{
		"print-err",
		janet_fastcgi_print_err,
		"(fastcgi/print-err text)\n\nPrint text to error.",
	},
	{
		"get-param",
		janet_fastcgi_get_param,
		"(fastcgi/get-param name)\n\nGet parameter value.",
	},
	{
		"read",
		janet_fastcgi_read,
		"(fastcgi/read max-length)\n\nRead up to max-length bytes from request body.",
	},
	{
		"read-all",
		janet_fastcgi_read_all,
		"(fastcgi/read-all)\n\nRead whole request body.",
	},
	{NULL, NULL, NULL},
};

JANET_MODULE_ENTRY(JanetTable *env) {
	janet_cfuns(env, "fastcgi", cfuns);
}
