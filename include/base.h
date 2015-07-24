/*
The MIT License (MIT)
Copyright (c) 2015 Tayfun Kayhan
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <glfw3.h>

#include <assert.h>

typedef unsigned int uint;

/* Declares an inlined getter function returning const varName field as const */
#define DECLARE_GETTER(type, concatName, varName) type get##concatName() const { return varName; }

/* Declares an inlined getter function returning non-const varName */
#define DECLARE_GETTER_NONCONST(type, concatName, varName) type get##concatName() { return varName; }

/* Declares an inlined setter function for varName as setconcatName(type val). Note the value is copied, not const referenced or ptr! */
#define DECLARE_SETTER_BY_VALUE(type, concatName, varName) void set##concatName(type val) { varName = val; }

/* Declares an inlined setter function with an argument passed by const reference to variable */
#define DECLARE_SETTER_BY_CONST_REF(type, concatName, varName) void set##concatName(const type& val) { varName = val; }

#define SAFE_DELETE(x) { if (x) { delete x; x = nullptr; } }
#define SAFE_DELETE_ARRAY(x) { if (x) delete[] x; x = nullptr; }

#if _DEBUG
#define LOG_ME(msg) printf("%s\n", msg)
#else
#define LOG_ME(msg) //printf("IGNORED")
#endif