/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2024 Yermalayeu Ihar.
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
#include "Cpl/Time.h"
#include "Cpl/Utils.h"
#include "Cpl/String.h"

#include <mutex>
#include <map>
#include <thread>
#include <memory>

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
    class PerformanceHistogram
    {
        typedef std::vector<uint64_t> Histogram;

        uint64_t _shift, _max;
        Histogram _histogram;

        CPL_INLINE void Expand()
        {
            size_t o = 0;
            for (size_t i = 0; i < _histogram.size(); i += 2, o += 1)
                _histogram[o] = _histogram[i + 0] + _histogram[i + 1];
            for (; o < _histogram.size(); o += 1)
                _histogram[0] = 0;
            _shift++;
            _max *= 2;
        }

    public:
        CPL_INLINE PerformanceHistogram(uint32_t size = 0)
            : _shift(0)
            , _max(size)
            , _histogram(AlignHi(size, 2), 0)
        {
        }

        CPL_INLINE PerformanceHistogram(const PerformanceHistogram& hs)
            : _shift(hs._shift)
            , _max(hs._max)
            , _histogram(hs._histogram)
        {
        }

        CPL_INLINE bool Enable() const
        {
            return _histogram.size() > 0;
        }

        CPL_INLINE void Add(uint64_t value)
        {
            while (value >= _max)
                Expand();
            _histogram[value >> _shift]++;
        }

        CPL_INLINE void Merge(const PerformanceHistogram& other)
        {
            assert(_histogram.size() == other._histogram.size());
            while (other._shift > _shift)
                Expand();
            size_t step = size_t(1) << (_shift - other._shift);
            for (size_t o = 0, t = 0; o < other._histogram.size(); t++, o += step)
            {
                for (size_t i = o, n = std::min(o + step, other._histogram.size()); i < n; ++i)
                    _histogram[t] += other._histogram[i];
            }
        }

        CPL_INLINE double Quantile(double quantile) const
        {
            quantile = std::max(0.0, std::min(quantile, 100.0));
            uint64_t total = 0, max = 0;
            for (size_t i = 0; i < _histogram.size(); i++)
                total += _histogram[i];
            uint64_t threshold = uint64_t(quantile * total / 100.0), lower = 0, upper = 0;
            size_t index = 0;
            for (; index < _histogram.size() && upper < threshold; index++, lower = upper, upper += _histogram[index]);
            uint64_t step = uint64_t(1) << _shift;
            if (index == _histogram.size())
                return Miliseconds(_histogram.size() * step);
            uint64_t value = index * step;
            if (upper > lower)
                value += (threshold - lower) * step / (upper - lower);
            return Miliseconds(value);
        }
    };

    //-----------------------------------------------------------------------------------------------------

    class PerformanceMeasurer
    {
        String	_name;
        int64_t _start, _current, _total, _min, _max;
        int64_t _count, _flop;
        bool _entered, _paused;
        PerformanceHistogram _histogram;

    public:
        CPL_INLINE PerformanceMeasurer(const String& name, int64_t flop = 0, uint32_t hist = 0)
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
            , _histogram(hist)
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
            , _histogram(pm._histogram)
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
                    if (_histogram.Enable())
                        _histogram.Add(_current);
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
            if (_histogram.Enable())
                _histogram.Merge(other._histogram);
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

        CPL_INLINE double Quantile(double quantile) const
        {
            return _count && _histogram.Enable() ? _histogram.Quantile(quantile) : 0.0;
        }

        CPL_INLINE double Total() const
        {
            return Miliseconds(_total);
        }

        CPL_INLINE size_t Count() const
        {
            return (size_t)_count;
        }

        CPL_INLINE String Name() const
        {
            return _name;
        }

        CPL_INLINE String ToStr() const
        {
            std::stringstream ss;
            ss << Cpl::ToStr(Total(), 0) << " ms" << " / " << Count() << " = " << Cpl::ToStr(Average(), 3) << " ms";
            ss << " {min = " << Cpl::ToStr(Min(), 3);
            ss << "; max = " << Cpl::ToStr(Max(), 3);
            if (_histogram.Enable())
            {
                ss << "; q50 = " << Cpl::ToStr(Quantile(50.0), 3);
                ss << "; q90 = " << Cpl::ToStr(Quantile(90.0), 3);
                ss << "; q99 = " << Cpl::ToStr(Quantile(99.0), 3);
            }
            ss << "}";
            if (_flop)
                ss << " " << Cpl::ToStr(GFlops(), 1) << " GFlops";
            return ss.str();
        }
    };

    //-----------------------------------------------------------------------------------------------------

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

    //-----------------------------------------------------------------------------------------------------

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

        CPL_INLINE PerformanceMeasurer* Get(const String& name, int64_t flop = 0, uint32_t hist = 0)
        {
            FunctionMap& thread = ThisThread();
            PerformanceMeasurer* pm = NULL;
            FunctionMap::iterator it = thread.find(name);
            if (it == thread.end())
            {
                pm = new PerformanceMeasurer(name, flop, hist);
                thread[name].reset(pm);
            }
            else
                pm = it->second.get();
            return pm;
        }

        CPL_INLINE PerformanceMeasurer* Get(const String func, const String& desc, int64_t flop = 0, uint32_t hist = 0)
        {
            return Get(func + "{ " + desc + " }", flop, hist);
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

        PerformanceMeasurer Merged(const String & name) const
        {
            PerformanceMeasurer merged(name);
            std::lock_guard<std::mutex> lock(_mutex);
            for (ThreadMap::const_iterator thread = _map.begin(); thread != _map.end(); ++thread)
            {
                FunctionMap::const_iterator function = thread->second.find(name);
                if (function != thread->second.end() && function->second->Average() != 0)
                {
                    if (merged.Average() == 0)
                        merged = *function->second;
                    else
                        merged.Merge(*function->second);
                }
            }
            return merged;
        }

        void Clear()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (ThreadMap::iterator thread = _map.begin(); thread != _map.end(); ++thread)
                thread->second.clear();
        }

        String Report() const
        {
            FunctionMap merged = Merged();
            std::stringstream report;
            for (FunctionMap::const_iterator function = merged.begin(); function != merged.end(); ++function)
            {
                const PerformanceMeasurer& pm = *function->second;
                if (pm.Count())
                    report << function->first << ": " << pm.ToStr() << std::endl;
            }
            return report.str();
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

#define CPL_PERF_FUNCFH(flop, hist) Cpl::PerformanceHolder CPL_CAT(__ph, __LINE__)(Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, (int64_t)(flop), (hist)))
#define CPL_PERF_FUNCF(flop) CPL_PERF_FUNCFH(flop, 0)
#define CPL_PERF_FUNC() CPL_PERF_FUNCFH(0, 0)

#define CPL_PERF_BEGFH(desc, flop, hist) Cpl::PerformanceHolder CPL_CAT(__ph, __LINE__)(Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc, (int64_t)(flop), (hist)))
#define CPL_PERF_BEGF(desc, flop) CPL_PERF_BEGFH(desc, flop, 0)
#define CPL_PERF_BEG(desc) CPL_PERF_BEGFH(desc, 0, 0)

#define CPL_PERF_IFFH(cond, desc, flop, hist) Cpl::PerformanceHolder CPL_CAT(__ph, __LINE__)((cond) ? Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc, (int64_t)(flop), (hist)) : NULL)
#define CPL_PERF_IFF(cond, desc, flop) CPL_PERF_IFFH(cond, desc, flop, 0)
#define CPL_PERF_IF(cond, desc) CPL_PERF_IFFH(cond, desc, 0, 0)

#define CPL_PERF_END(desc) Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc)->Leave();

#define CPL_PERF_INITFH(name, desc, flop, hist) Cpl::PerformanceHolder name(Cpl::PerformanceStorage::Global().Get(CPL_FUNCTION, desc, (int64_t)(flop), (hist)), false);
#define CPL_PERF_INITF(name, desc, flop) CPL_PERF_INITFH(name, desc, flop, 0);
#define CPL_PERF_INIT(name, desc) CPL_PERF_INITFH(name, desc, 0, 0);

#define CPL_PERF_START(name) name.Enter(); 
#define CPL_PERF_PAUSE(name) name.Leave(true);

#else

#define CPL_PERF_FUNCFH(flop, hist)
#define CPL_PERF_FUNCF(flop)
#define CPL_PERF_FUNC()

#define CPL_PERF_BEGFH(desc, flop, hist)
#define CPL_PERF_BEGF(desc, flop)
#define CPL_PERF_BEG(desc)

#define CPL_PERF_IFFH(cond, desc, flop, hist)
#define CPL_PERF_IFF(cond, desc, flop)
#define CPL_PERF_IF(cond, desc)

#define CPL_PERF_END(desc)

#define CPL_PERF_INITFH(name, desc, flop, hist)
#define CPL_PERF_INITF(name, desc, flop)
#define CPL_PERF_INIT(name, desc)

#define CPL_PERF_START(name)
#define CPL_PERF_PAUSE(name)

#endif
