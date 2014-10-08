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
    // when
    mach_vm_address_t demo_function_addr = lorgnette_lookup(self, "demo_function");
    mach_vm_address_t main_addr = lorgnette_lookup(self, "main");
    // then
    assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, "demo_function") == demo_function_addr);
    assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, "main") == main_addr);
}

void test_local_function_lookup_from_image(void)
{
    // given
    task_t self = mach_task_self();
    const char *image = progname;
    // when
    mach_vm_address_t simple_lookup = lorgnette_lookup(self, "main");
    mach_vm_address_t image_lookup = lorgnette_lookup_image(self, "main", image);
    // then
    assert(simple_lookup == image_lookup);
}

void test_local_sharedcached_function_lookup(void)
{
    // given
    task_t self = mach_task_self();
    // when
    mach_vm_address_t fprintf_addr = lorgnette_lookup(self, "fprintf");
    mach_vm_address_t dlopen_addr = lorgnette_lookup(self, "dlopen");
    // then
    assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, "fprintf") == fprintf_addr);
    assert((mach_vm_address_t)dlsym(RTLD_DEFAULT, "dlopen") == dlopen_addr);
}

void test_local_sharedcached_function_lookup_from_image(void)
{
    // given
    task_t self = mach_task_self();
    const char *fprintf_image = "libsystem_c.dylib";
//    const char *dlopen_image = "libdyld.dylib";
    // when
    mach_vm_address_t fprintf_simple_lookup = lorgnette_lookup(self, "fprintf");
    mach_vm_address_t fprintf_image_lookup = lorgnette_lookup_image(self, "fprintf", fprintf_image);
    // then
    assert(fprintf_simple_lookup == fprintf_image_lookup);
}


void test_local_linked_library_function_lookup(void)
{
    // given
    task_t self = mach_task_self();
    // when
    mach_vm_address_t curl_easy_init_addr = lorgnette_lookup(self, "curl_easy_init");
    // then
    assert((mach_vm_address_t)&curl_easy_init == curl_easy_init_addr);
}

#pragma mark - Remote tests

void test_remote_function_lookup(void)
{
    // given
    task_t target;
    int err = KERN_FAILURE;
    // when
    err = task_for_pid(mach_task_self(), 1 /* launchd */, &target);
    mach_vm_address_t remote_dlopen_addr = lorgnette_lookup(target, "dlopen");
    // then
    assert(err == KERN_SUCCESS);
    assert(remote_dlopen_addr > 0);
//    fprintf(stderr, "Remote dlopen @ %p, local @ %p\n", (void *)remote_dlopen_addr, dlsym(RTLD_DEFAULT, "dlopen"));
}


void test_remote_function_lookup_image(void)
{
    // given
    task_t target;
    const char *image = "libdyld.dylib";
    int err = KERN_FAILURE;
    // when
    err = task_for_pid(mach_task_self(), 1 /* launchd */, &target);
    mach_vm_address_t simple_remote_lookup = lorgnette_lookup(target, "dlopen");
    mach_vm_address_t remote_lookup_image = lorgnette_lookup_image(target, "dlopen", image);
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
