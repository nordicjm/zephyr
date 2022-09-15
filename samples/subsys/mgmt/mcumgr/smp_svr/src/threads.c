#include <zephyr/kernel.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(cto);

#define MY_STACK_SIZE_A 1024
#define MY_PRIORITY_A 2

#define MY_STACK_SIZE_B 1024
#define MY_PRIORITY_B 0

extern void my_entry_point_a(void *, void *, void *);
extern void my_entry_point_b(void *, void *, void *);

K_THREAD_STACK_DEFINE(my_stack_area_a, MY_STACK_SIZE_A);
struct k_thread my_thread_data_a;

K_THREAD_STACK_DEFINE(my_stack_area_b, MY_STACK_SIZE_B);
struct k_thread my_thread_data_b;

void setupthreads()
{
	k_tid_t my_tid_a = k_thread_create(&my_thread_data_a, my_stack_area_a, K_THREAD_STACK_SIZEOF(my_stack_area_a), my_entry_point_a, NULL, NULL, NULL, MY_PRIORITY_A, 0, K_NO_WAIT);
	k_tid_t my_tid_b = k_thread_create(&my_thread_data_b, my_stack_area_b, K_THREAD_STACK_SIZEOF(my_stack_area_b), my_entry_point_b, NULL, NULL, NULL, MY_PRIORITY_B, 0, K_NO_WAIT);

	k_thread_start(my_tid_a);
	k_thread_start(my_tid_b);
}


void my_entry_point_a(void *, void *, void *)
{
	while (1) {
		k_sleep(K_MSEC(66));
		LOG_ERR("test output a");
	}
}

void my_entry_point_b(void *, void *, void *)
{
	while (1) {
		k_sleep(K_MSEC(203));
		printk("test output b");
	}
}
