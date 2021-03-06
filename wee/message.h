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

#ifndef __WEE__MESSAGE_H__
#define __WEE__MESSAGE_H__

#include "list.h"
#include "object.h"

typedef struct msg_s
{
	OBJECT_HEADER;
	struct msg_s* next;
	char*         name;
	list_t*       arguments;
} msg_t;

// Primitive messages required by the runtime
extern msg_t* acute_forward_msg;
extern msg_t* acute_activate_msg;

// Create a new message
extern msg_t* msg_new(const char*, list_t*, msg_t*);

// Destroy a message
extern void msg_destroy(msg_t*);

// Perform a message
extern obj_t* msg_perform_on(msg_t*, obj_t*, obj_t*);

// Get the message at a specific index
extern msg_t* msg_arg_at(msg_t*, size_t);

// Eval arg at a specific index
extern obj_t* msg_eval_arg_at(msg_t*, obj_t*, size_t);

extern char* msg_string(msg_t*);

#endif /* !__WEE__MESSAGE_H__ */
