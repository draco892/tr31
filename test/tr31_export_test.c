/**
 * @file tr31_export_test.c
 *
 * Copyright (c) 2021 ono//connect
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "tr31.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// TR-31:2018, A.7.2.1
static const uint8_t test1_kbpk_raw[] = { 0x89, 0xE8, 0x8C, 0xF7, 0x93, 0x14, 0x44, 0xF3, 0x34, 0xBD, 0x75, 0x47, 0xFC, 0x3F, 0x38, 0x0C };
static const struct tr31_key_t test1_kbpk = {
	.usage = TR31_KEY_USAGE_TR31_KBPK,
	.algorithm = TR31_KEY_ALGORITHM_TDES,
	.mode_of_use = TR31_KEY_MODE_OF_USE_ENC_DEC,
	.length = sizeof(test1_kbpk_raw),
	.data = (void*)test1_kbpk_raw,
};
static const uint8_t test1_key_raw[] = { 0xED, 0xB3, 0x80, 0xDD, 0x34, 0x0B, 0xC2, 0x62, 0x02, 0x47, 0xD4, 0x45, 0xF5, 0xB8, 0xD6, 0x78 };
static const struct tr31_key_t test1_key = {
	.usage = TR31_KEY_USAGE_PIN,
	.algorithm = TR31_KEY_ALGORITHM_TDES,
	.mode_of_use = TR31_KEY_MODE_OF_USE_ENC,
	.key_version = TR31_KEY_VERSION_IS_UNUSED,
	.exportability = TR31_KEY_EXPORT_TRUSTED,
	.length = sizeof(test1_key_raw),
	.data = (void*)test1_key_raw,
};
static const char test1_tr31_header_verify[] = "A0072P0TE00E0000";
static const size_t test1_tr31_length_verify =
	16 /* header */
	+ 0 /* opt block */
	+ (2 /* key length */ + 16 /* key */ + 6 /* padding */) * 2
	+ (4 /* authenticator */) * 2;

// TR-31:2018, A.7.3.2
static const uint8_t test2_kbpk_raw[] = { 0x1D, 0x22, 0xBF, 0x32, 0x38, 0x7C, 0x60, 0x0A, 0xD9, 0x7F, 0x9B, 0x97, 0xA5, 0x13, 0x11, 0xAC };
static const struct tr31_key_t test2_kbpk = {
	.usage = TR31_KEY_USAGE_TR31_KBPK,
	.algorithm = TR31_KEY_ALGORITHM_TDES,
	.mode_of_use = TR31_KEY_MODE_OF_USE_ENC_DEC,
	.length = sizeof(test2_kbpk_raw),
	.data = (void*)test2_kbpk_raw,
};
static const uint8_t test2_key_raw[] = { 0xE8, 0xBC, 0x63, 0xE5, 0x47, 0x94, 0x55, 0xE2, 0x65, 0x77, 0xF7, 0x15, 0xD5, 0x87, 0xFE, 0x68 };
static const struct tr31_key_t test2_key = {
	.usage = TR31_KEY_USAGE_BDK,
	.algorithm = TR31_KEY_ALGORITHM_TDES,
	.mode_of_use = TR31_KEY_MODE_OF_USE_DERIVE,
	.key_version = TR31_KEY_VERSION_IS_VALID,
	.key_version_value = 12,
	.exportability = TR31_KEY_EXPORT_SENSITIVE,
	.length = sizeof(test2_key_raw),
	.data = (void*)test2_key_raw,
};
static const uint8_t test2_ksn[] = { 0x00, 0x60, 0x4B, 0x12, 0x0F, 0x92, 0x92, 0x80, 0x00, 0x00 };
static const char test2_tr31_header_verify[] = "B0104B0TX12S0100KS1800604B120F9292800000";
static const size_t test2_tr31_length_verify =
	16 /* header */
	+ 24 /* opt block */
	+ (2 /* key length */ + 16 /* key */ + 6 /* padding */) * 2
	+ (8 /* authenticator */) * 2;

// TR-31:2018, A.7.3.1
static const uint8_t test3_kbpk_raw[] = { 0xB8, 0xED, 0x59, 0xE0, 0xA2, 0x79, 0xA2, 0x95, 0xE9, 0xF5, 0xED, 0x79, 0x44, 0xFD, 0x06, 0xB9 };
static const struct tr31_key_t test3_kbpk = {
	.usage = TR31_KEY_USAGE_TR31_KBPK,
	.algorithm = TR31_KEY_ALGORITHM_TDES,
	.mode_of_use = TR31_KEY_MODE_OF_USE_ENC_DEC,
	.length = sizeof(test3_kbpk_raw),
	.data = (void*)test3_kbpk_raw,
};
static const uint8_t test3_key_raw[] = { 0xED, 0xB3, 0x80, 0xDD, 0x34, 0x0B, 0xC2, 0x62, 0x02, 0x47, 0xD4, 0x45, 0xF5, 0xB8, 0xD6, 0x78 };
static const struct tr31_key_t test3_key = {
	.usage = TR31_KEY_USAGE_BDK,
	.algorithm = TR31_KEY_ALGORITHM_TDES,
	.mode_of_use = TR31_KEY_MODE_OF_USE_DERIVE,
	.key_version = TR31_KEY_VERSION_IS_VALID,
	.key_version_value = 12,
	.exportability = TR31_KEY_EXPORT_SENSITIVE,
	.length = sizeof(test3_key_raw),
	.data = (void*)test3_key_raw,
};
static const uint8_t test3_ksn[] = { 0x00, 0x60, 0x4B, 0x12, 0x0F, 0x92, 0x92, 0x80, 0x00, 0x00 };
static const char test3_tr31_header_verify[] = "C0096B0TX12S0100KS1800604B120F9292800000";
static const size_t test3_tr31_length_verify =
	16 /* header */
	+ 24 /* opt block */
	+ (2 /* key length */ + 16 /* key */ + 6 /* padding */) * 2
	+ (4 /* authenticator */) * 2;

static void print_buf(const char* buf_name, const void* buf, size_t length)
{
	const uint8_t* ptr = buf;
	printf("%s: ", buf_name);
	for (size_t i = 0; i < length; i++) {
		printf("%02X", ptr[i]);
	}
	printf("\n");
}

int main(void)
{
	int r;
	struct tr31_ctx_t test_tr31;
	struct tr31_opt_ctx_t test_tr31_opts[1];
	char key_block[1024];

	// TR-31:2018, A.7.2.1
	printf("Test 1...\n");
	print_buf("kbpk", test1_kbpk_raw, sizeof(test1_kbpk_raw));
	test_tr31.version = TR31_VERSION_A;
	test_tr31.length = 72;
	test_tr31.key = test1_key;
	test_tr31.opt_blocks_count = 0;
	test_tr31.opt_blocks = NULL;

	r = tr31_export(&test_tr31, &test1_kbpk, key_block, sizeof(key_block));
	if (r) {
		fprintf(stderr, "tr31_export() failed; r=%d\n", r);
		goto exit;
	}
	printf("TR-31: %s\n", key_block);
	if (strncmp(key_block, test1_tr31_header_verify, strlen(test1_tr31_header_verify)) != 0) {
		fprintf(stderr, "TR-31 header encoding is incorrect\n");
		fprintf(stderr, "%s\n%s\n", key_block, test1_tr31_header_verify);
		r = 1;
		goto exit;
	}
	if (strlen(key_block) != test1_tr31_length_verify) {
		fprintf(stderr, "TR-31 length is incorrect\n");
		r = 1;
		goto exit;
	}

	// TR-31:2018, A.7.3.2
	printf("Test 2...\n");
	print_buf("kbpk", test2_kbpk_raw, sizeof(test2_kbpk_raw));
	test_tr31.version = TR31_VERSION_B;
	test_tr31.length = 104;
	test_tr31.key = test2_key;
	test_tr31.opt_blocks_count = 1;
	test_tr31.opt_blocks = test_tr31_opts;
	test_tr31.opt_blocks[0].id = TR31_OPT_BLOCK_KS;
	test_tr31.opt_blocks[0].data_length = sizeof(test2_ksn);
	test_tr31.opt_blocks[0].data = (void*)test2_ksn;

	r = tr31_export(&test_tr31, &test2_kbpk, key_block, sizeof(key_block));
	if (r) {
		fprintf(stderr, "tr31_export() failed; r=%d\n", r);
		goto exit;
	}
	printf("TR-31: %s\n", key_block);
	if (strncmp(key_block, test2_tr31_header_verify, strlen(test2_tr31_header_verify)) != 0) {
		fprintf(stderr, "TR-31 header encoding is incorrect\n");
		fprintf(stderr, "%s\n%s\n", key_block, test2_tr31_header_verify);
		r = 1;
		goto exit;
	}
	if (strlen(key_block) != test2_tr31_length_verify) {
		fprintf(stderr, "TR-31 length is incorrect\n");
		r = 1;
		goto exit;
	}

	// TR-31:2018, A.7.3.1
	printf("Test 3...\n");
	print_buf("kbpk", test3_kbpk_raw, sizeof(test3_kbpk_raw));
	test_tr31.version = TR31_VERSION_C;
	test_tr31.length = 96;
	test_tr31.key = test3_key;
	test_tr31.opt_blocks_count = 1;
	test_tr31.opt_blocks = test_tr31_opts;
	test_tr31.opt_blocks[0].id = TR31_OPT_BLOCK_KS;
	test_tr31.opt_blocks[0].data_length = sizeof(test3_ksn);
	test_tr31.opt_blocks[0].data = (void*)test3_ksn;

	r = tr31_export(&test_tr31, &test3_kbpk, key_block, sizeof(key_block));
	if (r) {
		fprintf(stderr, "tr31_export() failed; r=%d\n", r);
		goto exit;
	}
	printf("TR-31: %s\n", key_block);
	if (strncmp(key_block, test3_tr31_header_verify, strlen(test3_tr31_header_verify)) != 0) {
		fprintf(stderr, "TR-31 header encoding is incorrect\n");
		fprintf(stderr, "%s\n%s\n", key_block, test3_tr31_header_verify);
		r = 1;
		goto exit;
	}
	if (strlen(key_block) != test3_tr31_length_verify) {
		fprintf(stderr, "TR-31 length is incorrect\n");
		r = 1;
		goto exit;
	}

	printf("All tests passed.\n");
	r = 0;
	goto exit;

exit:
	return r;
}
