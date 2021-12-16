/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2021 Yermalayeu Ihar.
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

#include "Cpl/Defs.h"
#include "Cpl/String.h"

#include <mutex>
#include <map>
#include <thread>

#if defined(_MSC_VER)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__GNUC__)
#include <sys/time.h>
#else
#error Platform is not supported!
#endif

#if defined(CPL_PERF_ENABLE)
namespace Cpl
{
#if defined(_MSC_VER)
    CPL_INLINE int64_t TimeCounter()
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter.QuadPart;
    }

    CPL_INLINE int64_t TimeFrequency()
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return frequency.QuadPart;
    }
#elif defined(__GNUC__)
    CPL_INLINE int64_t TimeCounter()
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        return int64_t(t.tv_sec) * int64_t(1000000000) + int64_t(t.tv_nsec);
    }

    CPL_INLINE int64_t TimeFrequency()
    {
        return int64_t(1000000000);
    }
#else
#error Platform is not supported!
#endif

    CPL_INLINE double Seconds(int64_t count)
    {
        return double(count) / double(TimeFrequency());
    }

    CPL_INLINE double Miliseconds(int64_t count)
    {
        return double(count) / double(TimeFrequency()) * 1000.0;
    }

    CPL_INLINE double Time()
    {
        return double(TimeCounter()) / double(TimeFrequency());
    }

    //---------------------------------------------------------------------------------------------

    class PerformanceMeasurer
    {
        String	_name;
        int64_t _start, _current, _total, _min, _max;
        int64_t _count, _flop;
        bool _entered, _paused;

    public:
        CPL_INLINE PerformanceMeasurer(const String& name, int64_t flop = 0)
            : _name(name)
            , _flop(flop)
            , _count(0)
            , _start(0)
            , _current(0)
            , _total(0)
            , _min(std::numeric_limits<int64_t>::max())
            , _max(std::numeric_limits<int64_t>::min())
            , _entered(false)
            , _paused(false)
        {
        }

        CPL_INLINE PerformanceMeasurer(const PerformanceMeasurer& pm)
            : _name(pm._name)
            , _flop(pm._flop)
            , _count(pm._count)
            , _start(pm._start)
            , _current(pm._current)
            , _total(pm._total)
            , _min(pm._min)
            , _max(pm._max)
            , _entered(pm._entered)
            , _paused(pm._paused)
        {
        }

        CPL_INLINE void Enter()
        {
            if (!_entered)
            {
                _entered = true;
                _paused = false;
                _start = TimeCounter();
            }
        }

        CPL_INLINE void Leave(bool pause = false)
        {
            if (_entered || _paused)
            {
                if (_entered)
                {
                    _entered = false;
                    _current += TimeCounter() - _start;
                }
                if (!pause)
                {
                    _total += _current;
                    _min = std::min(_min, _current);
                    _max = std::max(_max, _current);
                    ++_count;
                    _current = 0;
                }
                _paused = pause;
            }
        }

        CPL_INLINE void Merge(const PerformanceMeasurer& other)
        {
            assert(_name == other._name);
            _count += other._count;
            _total += other._total;
            _min = std::min(_min, other._min);
            _max = std::max(_max, other._max);
        }

        CPL_INLINE double Average() const
        {
            return _count ? (Miliseconds(_total) / _count) : 0;
        }

        CPL_INLINE double GFlops() const
        {
            return _count && _flop && _total > 0 ? (double(_flop) * _count / Miliseconds(_total) / 1000000.0) : 0;
        }

        CPL_INLINE double Min() const
        {
            return _count ? Miliseconds(_min) : 0.0;
        }

        CPL_INLINE double Max() const
        {
            return _count ? Miliseconds(_max) : 0.0;
        }

        CPL_INLINE double Total() const
        {
            return Miliseconds(_total);
        }

        CPL_INLINE size_t Count() const
        {
            return (size_t)_count;
        }
    };

    //---------------------------------------------------------------------------------------------

    class PerformanceHolder
    {
        PerformanceMeasurer* _pm;

    public:
        inline PerformanceHolder(PerformanceMeasurer* pm, bool enter = true)
            : _pm(pm)
        {
            if (_pm && enter)
                _pm->Enter();
        }

        inline void Enter()
        {
            if (_pm)
                _pm->Enter();
        }

        inline void Leave(bool pause)
        {
            if (_pm)
                _pm->Leave(pause);
        }

        inline ~PerformanceHolder()
        {
            if (_pm)
                _pm->Leave();
        }
    };

    //---------------------------------------------------------------------------------------------

    class PerformanceStorage
    {
    public:
        typedef PerformanceMeasurer Pm;
        typedef std::shared_ptr<Pm> PmPtr;
        typedef std::map<String, PmPtr> FunctionMap;

        static PerformanceStorage s_storage;

        PerformanceStorage()
        {
        }

        CPL_INLINE PerformanceMeasurer* Get(const String& name, int64_t flop = 0)
        {
            FunctionMap& thread = ThisThread();
            PerformanceMeasurer* pm = NULL;
            FunctionMap::iterator it = thread.find(name);
            if (it == thread.end())
            {
                pm = new PerformanceMeasurer(name, flop);
                thread[name].reset(pm);
            }
            else
                pm = it->second.get();
            return pm;
        }

        CPL_INLINE PerformanceMeasurer* Get(const String func, const String& desc, int64_t flop = 0)
        {
            return Get(func + "{ " + desc + " }", flop);
        }

        FunctionMap Merged() const
        {
            FunctionMap merged;
            std::lock_guard<std::mutex> lock(_mutex);
            for (ThreadMap::const_iterator thread = _map.begin(); thread != _map.end(); ++thread)
            {
                for (FunctionMap::const_iterator function = thread->second.begin(); function != thread->second.end(); ++function)
                {
                    if (merged.find(function->first) == merged.end())
                        merged[function->first].reset(new PerformanceMeasurer(*function->second));
                    else
                        merged[function->first]->Merge(*function->second);
                }
            }
            return merged;
        }

        void Clear()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _map.clear();
        }

        static PerformanceStorage& Global()
        {
            static PerformanceStorage storage;
            return storage;
        }

    private:
        typedef std::map<std::thread::id, FunctionMap> ThreadMap;

        ThreadMap _map;
        mutable std::mutex _mutex;
        String _report;

        CPL_INLINE FunctionMap& ThisThread()
        {
            static thread_local FunctionMap* thread = NULL;
            if (thread == NULL)
            {
                std::lock_guard<std::mutex> lock(_mutex);
                thread = &_map[std::this_thread::get_id()];
            }
            return *thread;
        }
    };
}

#define CPL_PERF_FUNCF(flop) Cpl::PerformanceHolder CPL_CAT(__ph, __LINE__)(Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, (int64_t)(flop)))
#define CPL_PERF_FUNC() CPL_PERF_FUNCF(0)
#define CPL_PERF_BEGF(desc, flop) Cpl::PerformanceHolder CPL_CAT(__ph, __LINE__)(Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc, (int64_t)(flop)))
#define CPL_PERF_BEG(desc) CPL_PERF_BEGF(desc, 0)
#define CPL_PERF_IFF(cond, desc, flop) Cpl::PerformanceHolder CPL_CAT(__ph, __LINE__)((cond) ? Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc, (int64_t)(flop)) : NULL)
#define CPL_PERF_IF(cond, desc) CPL_PERF_IFF(cond, desc, 0)
#define CPL_PERF_END(desc) Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc)->Leave();
#define CPL_PERF_INITF(name, desc, flop) Cpl::PerformanceHolder name(Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc, (int64_t)(flop)), false);
#define CPL_PERF_INIT(name, desc)  CPL_PERF_INITF(name, desc, 0);
#define CPL_PERF_START(name) name.Enter(); 
#define CPL_PERF_PAUSE(name) name.Leave(true);

#else

#define CPL_PERF_FUNCF(flop)
#define CPL_PERF_FUNC()
#define CPL_PERF_BEGF(desc, flop)
#define CPL_PERF_BEG(desc)
#define CPL_PERF_IFF(cond, desc, flop)
#define CPL_PERF_IF(cond, desc)
#define CPL_PERF_END(desc)
#define CPL_PERF_INITF(name, desc, flop)
#define CPL_PERF_INIT(name, desc)
#define CPL_PERF_START(name)
#define CPL_PERF_PAUSE(name)

#endif
