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

obj_t* obj_new_with_size(size_t size)
{
    obj_t* obj = malloc(size);
    obj->slots = hash_new(DEFAULT_SLOTTABLE_SIZE);
    obj->cached_result = NULL;
    obj->shape = obj_shape(obj);
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
    obj->shape = obj_shape(obj);
    return success;
}

obj_t* obj_lookup_local(obj_t* obj, char* name)
{
    // First, lets look up a "lookup" slot, and use that instead of this if we have it.
    obj_t* lookup = hash_get(obj->slots, "lookup");

    if(!lookup)
        return hash_get(obj->slots, name);
    else
    {
        //TODO: Implement blocks, call lookup -- a block, with the appropriate arguments
        //      and return its value, whatever that may be.
    }

    return NULL;
}

obj_t* obj_lookup(obj_t* obj, char* name, obj_t** context)
{
    // Maintain a list of objects we've scanned, to avoid lookup loops.
    static list_t* objects_scanned = NULL;
    objects_scanned = list_new();

    if(list_contains(objects_scanned, obj))
        return NULL;

    obj_t* value = hash_get(obj->slots, name);
	fprintf(stderr, "value = %p\n", value);
    if(value || (value == NULL && list_contains(objects_scanned, obj)))
    {
        list_destroy(objects_scanned);
        return value;
    }
    // Now mark the object 'dirty' for our purposes. We've already scanned it.
    list_append(objects_scanned, obj);
    
    obj_t* parent = NULL;
    value = NULL;
    while((parent = hash_get(obj->slots, "parent")))
    {
        if(parent)
        {
            value = obj_lookup(parent, name, NULL);
            if(value == NULL)
                continue;
        }
    }
    //list_remove(objects_scanned, obj); // TODO: Implement
    *context = obj;
    list_destroy(objects_scanned);
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

    obj->shape = obj_shape(obj);

    return false;
}

obj_t* obj_perform(obj_t* target, obj_t* locals, msg_t* msg)
{
    obj_t* context = NULL;
    obj_t* obj = obj_lookup(target, msg->name, &context);
    if(obj && obj->cached_result)
        return obj->cached_result;

    if(obj)
    {
        block_t* activate = (block_t*)obj_lookup(obj, "activate", &context);
        if(activate)
            return block_activate(activate, target, locals, msg, context);
        else
            return obj;
    }
    obj = obj_lookup(target, "forward", &context);
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