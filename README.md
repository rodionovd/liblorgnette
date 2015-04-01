```
╦  ┌─┐┬─┐┌─┐┌┐┌┌─┐┌┬┐┌┬┐┌─┐   ┌┬┐┬ ┬┌─┐    ┬  ┬┌┐ 
║  │ │├┬┘│ ┬│││├┤  │  │ ├┤     │ ├─┤├┤     │  │├┴┐
╩═╝└─┘┴└─└─┘┘└┘└─┘ ┴  ┴ └─┘    ┴ ┴ ┴└─┘    ┴─┘┴└─┘

```
[![Build Status](https://travis-ci.org/rodionovd/liblorgnette.svg?branch=master)](https://travis-ci.org/rodionovd/liblorgnette)  

`Lorgnette` enables you to lookup symbols on OS X and iOS *(jailbreak is required, though)*. It works for both local and remote symbols (i.e. symbols within an alien process address space). So you may think about it as `dlsym()` without «the current process symbols only» and «won't find unexported symbols» limitations.

> OS X contains a private framework called `CoreSymbolication` that can be used to locate symbols in any running task on the system and even more. I believe that it has something to do with `dtrace`.   
So if you need something production-ready (heh), you should use the Apple thing instead of `liblorgnette`.  
See [`CoreSymbolication`](#coresymbolication) section of this file.  

### Usage

If the target is a `mach_task_self()` then `lorgnette_lookup()` will act like `dlsym()`. 
But unlike `dlsym()` it can be used to locate unexported symbols.  

```c
#include "lorgnette.h"
mach_vm_address_t main_addr = lorgnette_lookup(mach_task_self(), "main");
assert(dlsym(RTLD_DEFAULT, "main") == main_addr);

mach_vm_address_t dlopen_addr = lorgnette_lookup_image(mach_task_self(), "dlopen", "libdyld.dylib");
assert(dlsym(RTLD_DEFAULT, "dlopen") == dlopen_addr);
```

Or it will inspect any alien task you have rights to control (`task_for_pid` isn't
for everyone, you know):  

```c
pid_t proc = 20131;

task_t target;
task_for_pid(mach_task_self(), proc, &target);

mach_vm_address_t remote_addr = lorgnette_lookup(target, "_private_function");
```


### Interface  

**`mach_vm_address_t lorgnette_lookup(task_t target, const char *symbol_name);`**    

Locate a symbol inside an arbitrary process' address space.

> This function iterates *local symbols first* and only then it looks for symbols in
linked libraries.  

| Parameter   | Type (in/out) | Description |
| :--------: | :-----------: | :---------- |
| `target` | in  | _**(required)**_ The target process to inspect  |  
| `symbol_name` | in| _**(required)**_ The name of the symbol to find. This parameter must not be NULL  |  


| Return value  |  
| :---------- |   
| An address of the given symbol within the given process, or 0 (zero) if this symbol could not be found |  

----
----

**`mach_vm_address_t lorgnette_lookup_image(task_t target, const char *symbol_name, const char *image_name);`**    

Locate a symbol within a particular image inside an alien process.  

| Parameter   | Type (in/out) | Description |
| :--------: | :-----------: | :---------- |
| `target` | in  | _**(required)**_ The target process to inspect  |  
| `symbol_name` | in| _**(required)**_ The name of the symbol to find. This parameter must not be NULL  |  
| `image_name` | in | *(optional)* The name of the host image of the given symbol. The image name should be either a full file path or just a file base name  


| Return value  |  
| :---------- |   
| An address of the given symbol within the given process, or `0` (zero) if this symbol could not be found *[within the given image, if `image_name` is not NULL]* |  


## CoreSymbolication  

Here're some reverse engineered headers for the framework along with functionality tests: [mountainstorm/CoreSymbolication](https://github.com/mountainstorm/CoreSymbolication).  
Also see [this StackOverflow question](http://stackoverflow.com/questions/17445960/finding-offsets-of-local-symbols-in-shared-libraries-programmatically-on-os-x) by Johannes Weiß.

---------

If you found any bug(s) or something, please open an issue or a pull request — I'd appreciate your help! (^,,^)

Dmitry Rodionov, 2014  
i.am.rodionovd@gmail.com
