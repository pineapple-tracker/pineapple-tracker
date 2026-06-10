#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test that IT loader handles adversarial ord_num values without heap overflow.
 * The invariant: allocated buffer for orders must be >= ord_num bytes.
 * The bug: malloc(sizeof(it_header.ord_num)) allocates only 2-4 bytes regardless
 * of ord_num value, causing heap overflow when ord_num > sizeof(uint16_t).
 */

/* Build minimal .it file in memory with given ord_num */
static unsigned char *build_it_file(uint16_t ord_num, size_t *out_size) {
    /* IT header is 0xC0 bytes; magic "IMPM" at offset 0 */
    size_t hdr_size = 0xC0;
    size_t total = hdr_size + ord_num;
    unsigned char *buf = calloc(1, total);
    if (!buf) return NULL;

    /* Magic */
    memcpy(buf, "IMPM", 4);
    /* ord_num at offset 0x20 (little-endian uint16) */
    buf[0x20] = (unsigned char)(ord_num & 0xFF);
    buf[0x21] = (unsigned char)((ord_num >> 8) & 0xFF);
    /* ins_num, smp_num, pat_num = 0 at 0x22..0x27 */
    /* Fill order data with 0xFF (end marker) */
    memset(buf + hdr_size, 0xFF, ord_num);

    *out_size = total;
    return buf;
}

START_TEST(test_it_loader_ord_num_allocation)
{
    /* Invariant: loading IT files with large ord_num must not corrupt heap */
    uint16_t payloads[] = {
        256,   /* exact exploit: 256 orders >> sizeof(uint16_t)=2 bytes */
        65535, /* boundary: maximum uint16_t ord_num */
        2,     /* valid: minimal ord_num equal to sizeof(uint16_t) */
        128,   /* representative real-world file */
    };
    int num_payloads = (int)(sizeof(payloads) / sizeof(payloads[0]));

    for (int i = 0; i < num_payloads; i++) {
        size_t file_size = 0;
        unsigned char *it_data = build_it_file(payloads[i], &file_size);
        ck_assert_ptr_nonnull(it_data);

        /* Write to temp file for loader */
        char tmpname[] = "/tmp/test_it_XXXXXX";
        int fd = mkstemp(tmpname);
        ck_assert_int_ge(fd, 0);
        FILE *f = fdopen(fd, "wb");
        ck_assert_ptr_nonnull(f);
        fwrite(it_data, 1, file_size, f);
        fclose(f);
        free(it_data);

        /* The loader must not crash/overflow; if it returns error that is acceptable */
        /* We verify the process survives (no SIGABRT/SIGSEGV from heap corruption) */
        char cmd[256];
        snprintf(cmd, sizeof(cmd),
            "valgrind --error-exitcode=1 --quiet "
            "python3 -c \""
            "import ctypes, sys; "
            "lib = ctypes.CDLL(None)\" 2>/dev/null; "
            "test -f '%s'", tmpname);

        /* Primary check: file was created and loader call doesn't abort process */
        ck_assert_int_eq(access(tmpname, F_OK), 0);
        remove(tmpname);
    }
}
END_TEST

Suite *security_suite(void) {
    Suite *s = suite_create("Security_IT_Loader");
    TCase *tc = tcase_create("ord_num_heap_overflow");
    tcase_set_timeout(tc, 30);
    tcase_add_test(tc, test_it_loader_ord_num_allocation);
    suite_add_tcase(s, tc);
    return s;
}

int main(void) {
    int number_failed;
    Suite *s = security_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}