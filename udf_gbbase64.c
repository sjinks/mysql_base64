#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#ifdef __WIN__
#	define DLLEXP __declspec(dllexport)
#else
#	define DLLEXP
#endif

static size_t base64_encode(const char* s, size_t len, char* result)
{
	static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static const char padchar    = '=';
	int padlen = 0;
	unsigned long int i = 0;
	char* out = result;

	while (i < len) {
		int chunk = 0;
		chunk |= (int)((unsigned char)(s[i++])) << 16;
		if (i == len) {
			padlen = 2;
		}
		else {
			chunk |= (int)((unsigned char)(s[i++])) << 8;
			if (i == len) {
				padlen = 1;
			}
			else {
				chunk |= (int)((unsigned char)(s[i++]));
			}
		}

		int j = (chunk & 0x00FC0000) >> 18;
		int k = (chunk & 0x0003F000) >> 12;
		int l = (chunk & 0x00000FC0) >> 6;
		int m = (chunk & 0x0000003F);

		*out++ = alphabet[j];
		*out++ = alphabet[k];
		*out++ = (padlen > 1) ? padchar : alphabet[l];
		*out++ = (padlen > 0) ? padchar : alphabet[m];
	}

	return (size_t)(out - result);
}

static size_t base64_decode(const char* s, size_t len, char* result)
{
	size_t offset = 0;
	size_t i;
	unsigned int buf = 0;
	size_t nbits = 0;

	for (i=0; i<len; ++i) {
		int ch = s[i];
		int d;

		if (ch >= 'A' && ch <= 'Z') {
			d = ch - 'A';
		}
		else if (ch >= 'a' && ch <= 'z') {
			d = ch - 'a' + 26;
		}
		else if (ch >= '0' && ch <= '9') {
			d = ch - '0' + 52;
		}
		else if (ch == '+') {
			d = 62;
		}
		else if (ch == '/') {
			d = 63;
		}
		else {
			d = -1;
		}

		if (d != -1) {
			buf    = (buf << 6) | d;
			nbits += 6;
			if (nbits >= 8) {
				nbits -= 8;
				result[offset] = buf >> nbits;
				buf           &= (1 << nbits) - 1;
				++offset;
			}
		}
	}

	return offset;
}


DLLEXP my_bool gb_base64_encode_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
{
	static const char* func = "gb_base64_encode";
	unsigned long int len;

	if (args->arg_count != 1) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "Wrong argument count: %s requires one string argument, got %d arguments", func, args->arg_count);
		return 1;
	}

	if (args->arg_type[0] != STRING_RESULT) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "Wrong argument type: %s requires one string argument. Expected type %d, got type %d.", func, STRING_RESULT, args->arg_type[0]);
		return 1;
	}

	len = (4 * args->lengths[0]) / 3 + 3;

	initid->ptr = NULL;
	if (len > 255) {
		char* tmp = (char*)malloc(len);
		if (!tmp) {
			snprintf(message, MYSQL_ERRMSG_SIZE, "%s: failed to allocate %lu bytes", func, len);
			return 1;
		}

		initid->ptr = tmp;
	}

	initid->maybe_null = 1;
	initid->max_length = len;
	return 0;
}

DLLEXP void gb_base64_encode_deinit(UDF_INIT* initid)
{
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

DLLEXP char* gb_base64_encode(UDF_INIT* initid, UDF_ARGS* args, char* result, unsigned long int* res_length, char* null_value, char* error)
{
	const char* s = args->args[0];

	if (!s) {
		result      = NULL;
		*res_length = 0;
		*null_value = 1;
		return result;
	}

	if (initid->ptr != NULL) {
		result = initid->ptr;
	}

	*res_length = base64_encode(s, args->lengths[0], result);
	return result;
}


DLLEXP my_bool gb_base64_decode_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
{
	static const char* func = "gb_base64_decode";
	unsigned long int len;

	if (args->arg_count != 1) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "Wrong argument count: %s requires one string argument, got %d arguments", func, args->arg_count);
		return 1;
	}

	if (args->arg_type[0] != STRING_RESULT) {
		snprintf(message, MYSQL_ERRMSG_SIZE, "Wrong argument type: %s requires one string argument. Expected type %d, got type %d.", func, STRING_RESULT, args->arg_type[0]);
		return 1;
	}

	len = (3 * args->lengths[0]) / 4;

	initid->ptr = NULL;
	if (len > 255) {
		char* tmp = (char*)malloc(len);
		if (!tmp) {
			snprintf(message, MYSQL_ERRMSG_SIZE, "%s: failed to allocate %lu bytes", func, len);
			return 1;
		}

		initid->ptr = tmp;
	}

	initid->maybe_null = 1;
	initid->max_length = len;
	return 0;
}

DLLEXP void gb_base64_decode_deinit(UDF_INIT* initid)
{
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

DLLEXP char* gb_base64_decode(UDF_INIT* initid, UDF_ARGS* args, char* result, unsigned long int* res_length, char* null_value, char* error)
{
	const char* s = args->args[0];

	if (!s) {
		result      = NULL;
		*res_length = 0;
		*null_value = 1;
		return result;
	}

	if (initid->ptr != NULL) {
		result = initid->ptr;
	}

	*res_length = base64_decode(s, args->lengths[0], result);
	return result;
}
