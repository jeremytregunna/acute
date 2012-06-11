/*
 * Acute Programming Language
 * Copyright (c) 2011, Jeremy Tregunna, All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "list.h"
#include "slot.h"
#include "hash.h"
#include "message.h"
#include "object.h"
#include "block.h"

#define DEFAULT_SLOTTABLE_SIZE 8

obj_t* obj_new(void)
{
    return obj_new_with_size(sizeof(obj_t));
}

obj_t* obj_new_parent(obj_t* parent)
{
    obj_t* obj = obj_new_with_size(sizeof(*obj));
    obj_register_slot(obj, "parent", parent);
    obj_register_slot(obj, OBJ_STR(shape), obj_shape(obj));
    return obj;
}

obj_t* obj_new_with_size(size_t size)
{
    obj_t* obj = malloc(size);
    obj->slots = hash_new(DEFAULT_SLOTTABLE_SIZE);
    obj->cached_result = NULL;
    obj_register_slot_unsafe(obj, OBJ_STR(shape), obj_shape(obj));
    return obj;
}

void obj_destroy(obj_t* obj)
{
    hash_destroy(obj->slots);
    free(obj);
    obj = NULL;
}

bool obj_register_slot_unsafe(obj_t* obj, char* name, void* value)
{
    return hash_insert(obj->slots, name, value);
}

bool obj_register_slot(obj_t* obj, char* name, void* value)
{
    bool success = obj_register_slot_unsafe(obj, name, value);
    obj_register_slot_unsafe(obj, OBJ_STR(shape), obj_shape(obj));
    return success;
}

obj_t* obj_lookup_local(obj_t* obj, char* name)
{
    return hash_get(obj->slots, name);
}

obj_t* obj_lookup(obj_t* obj, msg_t* msg, obj_t** context)
{
    obj_t* value = obj_lookup_local(obj, msg->name);
	if(value)
    {
        if(context)
            *context = obj;
        return value;
    }

    block_t* forward = obj_lookup_local(obj, OBJ_STR(forward));
    if(forward)
        return block_call(forward, obj, msg);

    if(context)
        *context = obj;

    return value;
}

bool obj_use_trait(obj_t* obj, obj_t* trait, hash_t* resolutions)
{
    __block bool is_error = false;

    hash_foreach(trait->slots, ^(char* key, void* value) {
        if(obj_lookup_local(obj, key))
        {
            char* name = (char*)hash_get(resolutions, key);
            if(!name)
            {
                is_error = true;
                return; // Should raise an error here too. User hasn't handled conflicts
            }
            else
                obj_register_slot_unsafe(obj, name, value);
        }
        else
            obj_register_slot_unsafe(obj, key, value);
    });

    obj_register_slot(obj, OBJ_STR(shape), obj_shape(obj));

    return false;
}

obj_t* obj_perform(obj_t* target, obj_t* locals, msg_t* msg)
{
    obj_t* context = NULL;
    obj_t* obj = obj_lookup(target, msg, &context);

    if(obj)
    {
        if(obj->cached_result)
            return obj->cached_result;

        block_t* activate = (block_t*)obj_lookup(obj, acute_activate_msg, &context);
        if(activate)
            return block_activate(activate, target, locals, msg, context);
        else
            return obj;
    }

    obj = obj_lookup(target, acute_forward_msg, &context);
    if(obj)
        return block_call((block_t*)obj, locals, msg);
    return NULL;
}

uint32_t obj_shape(obj_t* obj)
{
    __block uint32_t hval = 0x811c9dc5;

    hash_foreach(obj->slots, ^(char* key, void* unused) {
        unsigned char* s = (unsigned char*)key;

        while(*s)
        {
            hval ^= (uint32_t)*s++;
            hval *= 0x01000193;
        }
    });

    return hval;
}

void obj_install_slots(obj_t* obj)
{

}

ACUTE_PRIM(obj, lookup)
{
    msg_t* name_msg = msg_arg_at(msg, 0);
    return obj_lookup(self, name_msg, NULL);
}