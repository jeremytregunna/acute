# Tasks

In no specific order, these are the tasks that are outstanding

* ~~Shapes - An object shape is defined as being a hash of the slot names of an object. A shape is like a loose type, and should be an integer. It must be recalculated on any modification to an object that affects the slot table.~~
* Simple shape checking on objects internally. Is that object we're getting out of `msg_eval_arg_at()` actually a block?
* CMake precompiled header integration. We depend on Clang and to have clang generate a precompiled header, we use the following: `clang -cc1 test.h -emit-pch -o test.h.pch`. Candidates for inclusion include:
** branch_prediction.h
* Garbage Collection — Baker's copying collector (preferred) or treadmill (acceptable) is desired for RTGC.
* Object Persistance - We need to be able to serialize all core types into in-memory versions of themselves, suitable for transmission over a wire. Additionally, this representation should be able to be written to a file.