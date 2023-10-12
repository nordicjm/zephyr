#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <string.h>

int8_t signed_a = -8;
int16_t signed_b = -25817;
int32_t signed_c = -257401848;
int64_t signed_d = -9223372036854775801;
uint8_t unsigned_a = 125;
uint16_t unsigned_b = 65512;
uint32_t unsigned_c = 29482049;
uint64_t unsigned_d = 9194958309820903814;
uint8_t array[] = "test text";
uint8_t not_an_int[3] = { 0x01, 0x04, 0x82 };

int test_handle_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg);
int test_handle_get(const char *name, char *val, int val_len_max);

struct settings_handler test_handler = {
                .name = "test",
                .h_get = test_handle_get,
                .h_set = test_handle_set,
};

int test_handle_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg)
{
        const char *next;
        size_t name_len;
        int rc;

        name_len = settings_name_next(name, &next);

        if (!next) {
                if (!strncmp(name, "signed_a", name_len)) {
                if (len != sizeof(signed_a)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &signed_a, sizeof(signed_a));
                        return 0;
                } else if (!strncmp(name, "signed_b", name_len)) {
                if (len > sizeof(signed_b)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &signed_b, sizeof(signed_b));
                        return 0;
                } else if (!strncmp(name, "signed_c", name_len)) {
                if (len > sizeof(signed_c)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &signed_c, sizeof(signed_c));
                        return 0;
                } else if (!strncmp(name, "signed_d", name_len)) {
                if (len > sizeof(signed_d)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &signed_d, sizeof(signed_d));
                        return 0;
                } else if (!strncmp(name, "unsigned_a", name_len)) {
                if (len > sizeof(unsigned_a)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &unsigned_a, sizeof(unsigned_a));
                        return 0;
                } else if (!strncmp(name, "unsigned_b", name_len)) {
                if (len > sizeof(unsigned_b)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &unsigned_b, sizeof(unsigned_b));
                        return 0;
                } else if (!strncmp(name, "unsigned_c", name_len)) {
                if (len > sizeof(unsigned_c)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &unsigned_c, sizeof(unsigned_c));
                        return 0;
                } else if (!strncmp(name, "unsigned_d", name_len)) {
                if (len > sizeof(unsigned_d)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, &unsigned_d, sizeof(unsigned_d));
                        return 0;
                } else if (!strncmp(name, "array", name_len)) {
                if (len > sizeof(array)) {
                                return -EINVAL;
                                }
                        rc = read_cb(cb_arg, array, sizeof(array));
                        return 0;
                } else if (!strncmp(name, "not_an_int", name_len)) {
                                if (len > sizeof(not_an_int)) {
                                return -EINVAL;
                                }

                        rc = read_cb(cb_arg, not_an_int, sizeof(not_an_int));
                        return 0;
                }
        }

        return -ENOENT;
}

int test_handle_get(const char *name, char *val, int val_len_max)
{
        const char *next;

        if (settings_name_steq(name, "signed_a", &next) && !next) {
            val_len_max = MIN(val_len_max, sizeof(signed_a));
                memcpy(val, &signed_a, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "signed_b", &next) && !next) {
        val_len_max = MIN(val_len_max, sizeof(signed_b));
                memcpy(val, &signed_b, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "signed_c", &next) && !next) {
        val_len_max = MIN(val_len_max, sizeof(signed_c));
                memcpy(val, &signed_c, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "signed_d", &next) && !next) {
        val_len_max = MIN(val_len_max, sizeof(signed_d));
                memcpy(val, &signed_d, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "unsigned_a", &next) && !next) {
        val_len_max = MIN(val_len_max, sizeof(unsigned_a));
                memcpy(val, &unsigned_a, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "unsigned_b", &next) && !next) {
        val_len_max = MIN(val_len_max, sizeof(unsigned_b));
                memcpy(val, &unsigned_b, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "unsigned_c", &next) && !next) {
        val_len_max = MIN(val_len_max, sizeof(unsigned_c));
                memcpy(val, &unsigned_c, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "unsigned_d", &next) && !next) {
        val_len_max = MIN(val_len_max, sizeof(unsigned_d));
                memcpy(val, &unsigned_d, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "array", &next) && !next) {
        val_len_max = MIN(val_len_max, strnlen(array, sizeof(array)));
                memcpy(val, array, val_len_max);
                return val_len_max;
        } else if (settings_name_steq(name, "not_an_int", &next) && !next) {
                val_len_max = MIN(val_len_max, sizeof(not_an_int));
                memcpy(val, not_an_int, val_len_max);
                return val_len_max;
        }

        return -ENOENT;
}

void do_app_settings()
{
        (void)settings_register(&test_handler);
}
