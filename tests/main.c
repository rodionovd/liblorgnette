//
//  main.c
//  lorgnette_demo
//
//  Created by Dmitry Rodionov on 9/24/14.
//  Copyright (c) 2014 rodionovd. All rights reserved.
//
#include <stdio.h>
#include <assert.h>
/* For tests */
#include <dlfcn.h>
#include <libgen.h>
#include <curl/curl.h>

#include "lorgnette.h"

const char *progname = NULL;

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
    const char *image = progname;
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
     * if our host is x86_64 too. */
    assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, symbol) == lookup);
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
    progname = basename((char *)argv[0]);
    
    /* Local tests */
    test_local_function_lookup();
    test_local_function_lookup_from_image();
    test_local_sharedcached_function_lookup();
    test_local_sharedcached_function_lookup_from_image();
    test_local_linked_library_function_lookup();

    /* Remote tests */
    test_remote_function_lookup();
    test_remote_function_lookup_image();
    
    return 0;
}
