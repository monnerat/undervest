//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Event scheduler class.                 *
//               *                                           *
//               *-------------------------------------------*
//               *                                           *
//               * Copyright (c) 2014-2015 Datasphere S.A.   *
//               *                                           *
//               *   This software is licensed as described  *
//               * in the file LICENSE, which you should     *
//               * have received as part of this             *
//               * distribution.                             *
//               *   You may opt to use, copy, modify,       *
//               * merge, publish, distribute and/or sell    *
//               * copies of this software, and permit       *
//               * persons to whom this software is          *
//               * furnished to do so, under the terms of    *
//               * the LICENSE file.                         *
//               *   This software is distributed on an      *
//               * "AS IS" basis, WITHOUT WARRANTY OF ANY    *
//               * KIND, either express or implied.          *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_SCHEDULER_H_
#define __INCLUDE_SCHEDULER_H_

#include <queue>
#include <vector>
#include <functional>
#include <utility>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>


class Scheduler {

public:
	typedef std::chrono::high_resolution_clock	Clock;
	typedef std::chrono::time_point<Clock>		Time;
	typedef Time::duration				Duration;
	class TimedEvent;

private:
	struct _Impl_base;
	typedef std::shared_ptr<_Impl_base>	_shared_base_type;

	struct _Impl_base {
		_shared_base_type	_ptr;

		inline virtual ~_Impl_base();
		virtual void _run(Scheduler& sched, TimedEvent te) = 0;
	};

	template<typename _Callable>
	struct _Impl: public _Impl_base {
		_Callable	_func;

		_Impl(_Callable&& _f): _func(std::forward<_Callable>(_f)) {};

		void
		_run(Scheduler& sched, TimedEvent te)
		{
			_func(sched, te);
		}
	};

	template<typename _Callable>
	static std::shared_ptr<_Impl<_Callable>>
	_make_routine(_Callable&& f)
	{
		return std::make_shared<_Impl<_Callable>>(
		    std::forward<_Callable>(f));
	}

	class _TimedEventData;
	typedef std::shared_ptr<_TimedEventData>	_TE;

	class _TimedEventData: public Time {

		friend class Scheduler;

	private:
		Scheduler *		_qued;
		bool			_cancelled;
		_shared_base_type	_func;

	public:
		_TimedEventData():
		    Time(), _qued(0), _cancelled(false), _func() {};

		_TimedEventData(const _TE& s):
		    Time(*s), _qued(0), _cancelled(false), _func(s->_func) {};

		template<typename _Callable, typename... _Args>
		_TimedEventData(const Time& t, _Callable&& f, _Args&&... args):
		    Time(t), _qued(0), _cancelled(false),
		    _func(_make_routine(std::bind(std::forward<_Callable>(f),
		    std::forward<_Args>(args)...,
		    std::placeholders::_1, std::placeholders::_2))) {};

		template<typename _Callable, typename... _Args>
		_TimedEventData(const Duration& d,
					_Callable&& f, _Args&&... args):
		    Time(Clock::now() + d), _qued(0), _cancelled(false),
		    _func(_make_routine(std::bind(std::forward<_Callable>(f),
		    std::forward<_Args>(args)...,
		    std::placeholders::_1, std::placeholders::_2))) {};

		void cancel() { if (_qued) _cancelled = true; };

		void run(Scheduler& sched, TimedEvent te)
		{
			_func->_run(sched, te);
		};
	};

public:
	class TimedEvent: public _TE {

		friend class Scheduler;

	public:
		TimedEvent() = default;

		TimedEvent(const TimedEvent& s): _TE()
		{
			static_cast<_TE&>(*this) = s;
		}

		template<typename... _Args>
		explicit
		TimedEvent(_Args&&... args): _TE(std::make_shared
		    <_TimedEventData>(std::forward<_Args>(args)...)) {};

		void run(Scheduler& sched)
		{
			(*this)->run(sched, *this);
		};

		void cancel() { (*this)->cancel(); };
	};

	bool
	schedule(TimedEvent te, const Time * t = NULL)
	{
		std::unique_lock<std::mutex> ulock(_exclude);

		if (te->_qued)
			return true;				// Fails.

		te->_qued = this;
		te->_cancelled = false;

		if (t)
			static_cast<Time&>(*te) = *t;

		_queue.push(te);

		if (static_cast<Time>(*te) <= static_cast<Time>(*_queue.top()))
			_notEmpty.notify_one();

		return false;
	}

	bool
	schedule(TimedEvent te, const Time& t)
	{
		return schedule(te, &t);
	}

	bool
	reschedule(TimedEvent te, const Time& t)
	{
		return schedule(te, &t);
	}

	bool
	reschedule(TimedEvent te, const Duration& d)
	{
		Time t(Clock::now() + d);

		return schedule(te, &t);
	}

	void
	run()
	{
		Time curtime;
		TimedEvent event;
		std::unique_lock<std::mutex> ulock(_exclude);

		for (_stop = false; !_stop;) {
			if (_queue.empty())
				_notEmpty.wait(ulock);
			else {
				curtime = Clock::now();
				event = _queue.top();

				if (curtime < *event)
					_notEmpty.wait_until(ulock, *event);
				else {
					_queue.pop();
					event->_qued = 0;

					if (!event->_cancelled) {
						ulock.unlock();
						event.run(*this);
						ulock.lock();
						}
					}
				}
			}
	}

	void stop()
	{
		_stop = true;

		{
			std::unique_lock<std::mutex> ulock(_exclude);

			_notEmpty.notify_one();
		}
	}

	void
	clear()
	{
		std::unique_lock<std::mutex> ulock(_exclude);
		TimedEvent event;

		while(!_queue.empty()) {
			event = _queue.top();
			event->_qued = 0;
			_queue.pop();
			}
	}

private:
	struct TimedEventGreater {
		bool operator()(const TimedEvent& p1, const TimedEvent& p2)
		{
			return *p1 > *p2;
		}
	};

	std::priority_queue<TimedEvent, std::vector<TimedEvent>,
	    TimedEventGreater>		_queue;
	std::mutex			_exclude;
	std::condition_variable		_notEmpty;
	bool				_stop;
};


inline Scheduler::_Impl_base::~_Impl_base() = default;

#endif
