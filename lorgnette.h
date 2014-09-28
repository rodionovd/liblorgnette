//
//  lorgnette.h
//  liblorgnette
//
//  Created by Dmitry Rodionov on 9/24/14.
//  Copyright (c) 2014 rodionovd. All rights reserved.
//
#include <mach/mach.h>
#pragma once
/**
 * @abstract
 * Look up a symbol adress inside an arbitrary process' address space.
 *
 * @note
 * This function iterates local symbols first and only then it looks for symbols
 * in linked libraries.
 *
 * @param target
 * The target process to inspect.
 * @param symbol_name
 * The name of the symbol to find. This parameter must not be NULL.
 *
 * @return
 * An address of the given symbol within the given process, or 0 (zero) if this symbol
 * could not be found.
 *
 * @b Examples
 *
 * Find a @p dlopen symbol address within the current task memory space
 * @code
 * addr = lorgnette_lookup(mach_task_self(), "dlopen");
 * @endcode
 * Find a @p glob_var0 symbol address inside a remote process
 * @code
 * addr = lorgnette_lookup(some_task, "glob_var0");
 * @endcode
 */
mach_vm_address_t lorgnette_lookup(task_t target, const char *symbol_name);
