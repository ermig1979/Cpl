/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once

//#define CPL_TEST_NORETURN

#include "Cpl/Defs.h"
#include "Cpl/Args.h"
#include "Cpl/File.h"
#include "Cpl/Log.h"
#include "Cpl/Performance.h"

#include <functional>

namespace Test
{
    typedef Cpl::Log Log;
    typedef Cpl::Int Int;
    typedef Cpl::String String;
    typedef Cpl::Strings Strings;

    /*!
    * Runs body in a child process (fork on POSIX) so that a crash, a sanitizer abort or an endless loop
    * inside body is reported as a test failure instead of taking the whole test application down.
    * Returns false when body returns false, terminates abnormally or does not finish within timeoutMs.
    * On platforms without fork the body is called directly: an exception thrown by it is still reported
    * as a failure, but the timeout has no effect and a crash takes the whole test application down.
    */
    bool RunIsolated(const std::function<bool()>& body, size_t timeoutMs = 3000);
}

#include "Test/TestOptions.h"
