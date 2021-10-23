/**
 * @file tr31-tool.c
 *
 * Copyright (c) 2020, 2021 ono//connect
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "tr31.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <argp.h>

// global parameters
static size_t key_block_len = 0;
static const char* key_block = NULL;
static size_t kbpk_buf_len = 0;
static uint8_t kbpk_buf[32]; // max 256-bit KBPK

// helper functions
static error_t argp_parser_helper(int key, char* arg, struct argp_state* state);
static int parse_hex(const char* hex, void* bin, size_t bin_len);
static void print_hex(const void* buf, size_t length);

// argp option structure
static struct argp_option argp_options[] = {
	{ "key-block", 'i', "asdf", 0, "TR-31 key block input" },
	{ "kbpk", 'k', "key", 0, "TR-31 key block protection key value (hex encoded)" },
	{ "version", 'v', NULL, 0, "Display TR-31 library version" },
	{ 0 },
};

// argp configuration
static struct argp argp_config = {
	argp_options,
	argp_parser_helper,
	NULL,
	NULL,
};

// argp parser helper function
static error_t argp_parser_helper(int key, char* arg, struct argp_state* state)
{
	int r;

	switch (key) {
		case 'i':
			key_block = arg;
			key_block_len = strlen(arg);
			return 0;

		case 'k':
			if (strlen(arg) > sizeof(kbpk_buf) * 2) {
				argp_error(state, "kbpk string may not have more than %zu digits (thus %zu bytes)", sizeof(kbpk_buf) * 2, sizeof(kbpk_buf));
			}
			if (strlen(arg) % 2 != 0) {
				argp_error(state, "kbpk string must have even number of digits");
			}
			kbpk_buf_len = strlen(arg) / 2;

			r = parse_hex(arg, kbpk_buf, kbpk_buf_len);
			if (r) {
				argp_error(state, "kbpk string must must consist of hex digits");
			}

			return 0;

		case 'v': {
			const char* version;

			version = tr31_lib_version_string();
			if (version) {
				printf("%s\n", version);
			} else {
				printf("Unknown\n");
			}
			exit(EXIT_SUCCESS);
			return 0;
		}

		default:
			return ARGP_ERR_UNKNOWN;
	}
}

// hex parser helper function
static int parse_hex(const char* hex, void* bin, size_t bin_len)
{
	size_t hex_len = bin_len * 2;

	for (size_t i = 0; i < hex_len; ++i) {
		if (!isxdigit(hex[i])) {
			return -1;
		}
	}

	while (*hex && bin_len--) {
		uint8_t* ptr = bin;

		char str[3];
		strncpy(str, hex, 2);
		str[2] = 0;

		*ptr = strtoul(str, NULL, 16);

		hex += 2;
		++bin;
	}

	return 0;
}

// hex output helper function
static void print_hex(const void* buf, size_t length)
{
	const uint8_t* ptr = buf;
	for (size_t i = 0; i < length; i++) {
		printf("%02X", ptr[i]);
	}
}

int main(int argc, char** argv)
{
	int r;
	struct tr31_key_t kbpk;
	struct tr31_ctx_t tr31_ctx;

	if (argc == 1) {
		// No command line options
		argp_help(&argp_config, stdout, ARGP_HELP_STD_HELP, argv[0]);
		return 1;
	}

	// parse command line options
	r = argp_parse(&argp_config, argc, argv, 0, 0, 0);
	if (r) {
		fprintf(stderr, "Failed to parse command line\n");
		return 1;
	}

	// populate key block protection key
	memset(&kbpk, 0, sizeof(kbpk));
	kbpk.usage = TR31_KEY_USAGE_TR31_KBPK;
	kbpk.mode_of_use = TR31_KEY_MODE_OF_USE_ENC_DEC;
	kbpk.length = kbpk_buf_len;
	kbpk.data = kbpk_buf;

	// determine key block protection key algorithm from keyblock format version
	switch (key_block[0]) {
		case TR31_VERSION_A:
		case TR31_VERSION_B:
		case TR31_VERSION_C:
			kbpk.algorithm = TR31_KEY_ALGORITHM_TDES;
			break;

		case TR31_VERSION_D:
			kbpk.algorithm = TR31_KEY_ALGORITHM_AES;
			break;

		default:
			fprintf(stderr, "%s\n", tr31_get_error_string(TR31_ERROR_UNSUPPORTED_VERSION));
			return 1;
	}

	if (kbpk_buf_len) { // if key block protection key was provided
		// parse and decrypt TR-31 key block
		r = tr31_import(key_block, &kbpk, &tr31_ctx);
	} else { // else if no key block protection key was provided
		// parse TR-31 key block
		r = tr31_import(key_block, NULL, &tr31_ctx);
	}
	// check for errors
	if (r) {
		fprintf(stderr, "TR-31 import error %d: %s\n", r, tr31_get_error_string(r));
		// continue to print key block details
	}

	// print key block details
	char ascii_buf[3]; // temporary ascii buffer
	printf("Key block format version: %c\n", tr31_ctx.version);
	printf("Key block length: %zu bytes\n", tr31_ctx.length);
	printf("Key usage: [%s] %s\n",
		tr31_get_key_usage_ascii(tr31_ctx.key.usage, ascii_buf, sizeof(ascii_buf)),
		tr31_get_key_usage_string(tr31_ctx.key.usage)
	);
	printf("Key algorithm: [%c] %s\n",
		tr31_ctx.key.algorithm,
		tr31_get_key_algorithm_string(tr31_ctx.key.algorithm)
	);
	printf("Key mode of use: [%c] %s\n",
		tr31_ctx.key.mode_of_use,
		tr31_get_key_mode_of_use_string(tr31_ctx.key.mode_of_use)
	);
	switch (tr31_ctx.key.key_version) {
		case TR31_KEY_VERSION_IS_UNUSED: printf("Key version: Unused\n"); break;
		case TR31_KEY_VERSION_IS_VALID: printf("Key version: %u\n", tr31_ctx.key.key_version_value); break;
		case TR31_KEY_VERSION_IS_COMPONENT: printf("Key component: %u\n", tr31_ctx.key.key_component_number); break;
	}
	printf("Key exportability: [%c] %s\n",
		tr31_ctx.key.exportability,
		tr31_get_key_exportability_string(tr31_ctx.key.exportability)
	);

	// print optional blocks, if available
	if (tr31_ctx.opt_blocks_count) {
		printf("Optional blocks [%zu]:\n", tr31_ctx.opt_blocks_count);
	}
	if (tr31_ctx.opt_blocks) { // might be NULL when tr31_import() fails
		for (size_t i = 0; i < tr31_ctx.opt_blocks_count; ++i) {
			const char* opt_block_data_str;

			printf("\t[%s] %s: ",
				tr31_get_opt_block_id_ascii(tr31_ctx.opt_blocks[i].id, ascii_buf, sizeof(ascii_buf)),
				tr31_get_opt_block_id_string(tr31_ctx.opt_blocks[i].id)
			);
			print_hex(tr31_ctx.opt_blocks[i].data, tr31_ctx.opt_blocks[i].data_length);

			opt_block_data_str = tr31_get_opt_block_data_string(&tr31_ctx.opt_blocks[i]);
			if (opt_block_data_str) {
				printf(" (%s)", opt_block_data_str);
			}

			printf("\n");
		}
	}

	// if available, print decrypted key
	if (tr31_ctx.key.length) {
		if (tr31_ctx.key.data) {
			printf("Key length: %zu\n", tr31_ctx.key.length);
			printf("Key value: ");
			print_hex(tr31_ctx.key.data, tr31_ctx.key.length);
			printf(" (KCV: ");
			print_hex(tr31_ctx.key.kcv, sizeof(tr31_ctx.key.kcv));
			printf(")\n");
		} else {
			printf("Key decryption failed\n");
		}
	} else {
		printf("Key not decrypted\n");
	}

	// cleanup
	tr31_release(&tr31_ctx);
}
