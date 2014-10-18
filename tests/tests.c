//
//  tests.c
//  liblorgnette
//
#include <stdio.h>
#include <assert.h>
#include <dlfcn.h>
#include <libgen.h>
#include <curl/curl.h>
#include "../lorgnette.h"

extern char ***_NSGetArgv(void);

#pragma mark - Local tests

void test_local_function_lookup(void)
{
	// given
	task_t self = mach_task_self();
	const char *image = "demo_function";
	// when
	mach_vm_address_t lookup = lorgnette_lookup(self, image);
	// then
	assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, image) == lookup);
}

void test_local_function_lookup_from_image(void)
{
	// given
	task_t self = mach_task_self();
	const char *symbol = "main";
	const char *image = basename((*_NSGetArgv())[0]);
	// when
	mach_vm_address_t simple_lookup = lorgnette_lookup(self, symbol);
	mach_vm_address_t image_lookup = lorgnette_lookup_image(self, symbol, image);
	// then
	assert(simple_lookup == image_lookup);
}

void test_local_sharedcached_function_lookup(void)
{
	// given
	task_t self = mach_task_self();
	const char *symbol = "fprintf";
	// when
	mach_vm_address_t lookup = lorgnette_lookup(self, symbol);
	// then
	assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, symbol) == lookup);
}

void test_local_sharedcached_function_lookup_from_image(void)
{
	// given
	task_t self = mach_task_self();
	const char *symbol = "fprintf";
	const char *image = "libsystem_c.dylib";
	// when
	mach_vm_address_t simple_lookup = lorgnette_lookup(self, symbol);
	mach_vm_address_t image_lookup = lorgnette_lookup_image(self, symbol, image);
	// then
	assert(simple_lookup == image_lookup);
}

void test_local_linked_library_function_lookup(void)
{
	// given
	task_t self = mach_task_self();
	const char *symbol = "curl_easy_init";
	// when
	mach_vm_address_t lookup = lorgnette_lookup(self, symbol);
	// then
	assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, symbol) == lookup);
}

/* As for OS X 10.9 there're two system libraries that contain _pthread_set_self symbol:
 * (1) libsystem_kernel.dylib and (2) libsystem_pthread.dylib.
 * In the former one is a no-op function while the latter holds the real symbol.
 *
 * Since libsystem_kernel.dylib gets loaded *before* libsystem_pthread.dylib,
 * lorgnette_lookup() consumes its (no-op) variant of _pthread_set_self, so if we want to
 * get a real function address, we must specify the right image. */
void test_pthread_set_self_lookup_image(void)
{
	// given
	task_t self = mach_task_self();
	const char *symbol = "_pthread_set_self";
	const char *image_pthread = "libsystem_pthread.dylib";
	const char *image_kernel  = "libsystem_kernel.dylib";
	// when
	mach_vm_address_t simple_lookup = lorgnette_lookup(self, symbol);
	mach_vm_address_t kernel_image_lookup  = lorgnette_lookup_image(self, symbol, image_kernel);
	mach_vm_address_t pthread_image_lookup = lorgnette_lookup_image(self, symbol, image_pthread);
	// then
	assert(simple_lookup != pthread_image_lookup);
	assert(simple_lookup == kernel_image_lookup);
	assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, symbol) == pthread_image_lookup);
}

#pragma mark - Remote tests

void test_remote_function_lookup(void)
{
	// given
	task_t target;
	const char *symbol = "dlopen";
	int err = KERN_FAILURE;
	// when
	err = task_for_pid(mach_task_self(), 1 /* launchd */, &target);
	mach_vm_address_t lookup = lorgnette_lookup(target, symbol);
	// then
	assert(err == KERN_SUCCESS);
	assert(lookup > 0);
#if defined(__x86_64__)
	/* Since libdyld is a part of dyld shared cache, both local
	 * and remote addresses should be the same.
	 * launchd is x86_64 on any modern OS, so we only perform this check
	 * if our host is x86_64 too.
	 *
	 * UPDATE:
	 * As it turns out there *may not be* any shared cache at all, so this
	 * assert will fail.
	 */
	// assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, symbol) == lookup);
#endif
}

void test_remote_function_lookup_image(void)
{
	// given
	task_t target;
	const char *symbol = "dlopen";
	const char *image  = "libdyld.dylib";
	int err = KERN_FAILURE;
	// when
	err = task_for_pid(mach_task_self(), 1 /* launchd */, &target);
	mach_vm_address_t simple_remote_lookup = lorgnette_lookup(target, symbol);
	mach_vm_address_t remote_lookup_image  = lorgnette_lookup_image(target, symbol, image);
	// then
	assert(err == KERN_SUCCESS);
	assert(simple_remote_lookup == remote_lookup_image);
}

#pragma mark - Misc

void __attribute__ ((visibility ("default")))
demo_function(void)
{
	printf("%s", __FUNCTION__);
}

int __attribute__ ((visibility ("default")))
main(int argc, const char * argv[])
{
	/* Local tests */
	test_local_function_lookup();
	test_local_function_lookup_from_image();
	test_local_sharedcached_function_lookup();
	test_local_sharedcached_function_lookup_from_image();
	test_local_linked_library_function_lookup();
	test_pthread_set_self_lookup_image();

	/* Remote tests */
	test_remote_function_lookup();
	test_remote_function_lookup_image();

	return 0;
}
