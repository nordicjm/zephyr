#define DEFAULT_FOO_VAL_VALUE 0

static uint8_t foo_val = DEFAULT_FOO_VAL_VALUE;

int foo_settings_get(const char *name, char *val, int val_len_max)
{
LOG_ERR("get?");
        const char *next;

        if (settings_name_steq(name, "bar", &next) && !next) {
                val_len_max = MIN(val_len_max, sizeof(foo_val));
                memcpy(val, &foo_val, val_len_max);
                return val_len_max;
        }

        return -ENOENT;
}

static int foo_settings_set(const char *name, size_t len,
                            settings_read_cb read_cb, void *cb_arg)
{
LOG_ERR("set?");
    const char *next;
    int rc;

    if (settings_name_steq(name, "bar", &next) && !next) {
        if (len != sizeof(foo_val)) {
            return -EINVAL;
        }

        rc = read_cb(cb_arg, &foo_val, sizeof(foo_val));
        if (rc >= 0) {
            return 0;
        }

        return rc;
    }


    return -ENOENT;
}

int alpha_handle_commit(void)
{
LOG_ERR("commit?");
return 0;
}

int alpha_handle_export(int (*cb)(const char *name,
			       const void *value, size_t val_len))
{
LOG_ERR("export?");
	(void)cb("foo/bar", &foo_val, sizeof(foo_val));
return 0;
}

struct settings_handler my_conf = {
    .name = "foo",
    .h_get = foo_settings_get,
    .h_set = foo_settings_set,

.h_commit = alpha_handle_commit,
.h_export = alpha_handle_export
};


void blah()
{
#if 1
    settings_subsys_init();
    settings_register(&my_conf);
    settings_load();
#endif

LOG_ERR("this value is %d", foo_val);
}
