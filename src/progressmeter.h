//  This file is part of par2cmdline (a PAR 2.0 compatible file verification and
//  repair tool). See http://parchive.sourceforge.net for details of PAR 2.0.
//
//  Copyright (c) 2003 Peter Brian Clements
//
//  par2cmdline is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  par2cmdline is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef __PROGRESSMETER_H__
#define __PROGRESSMETER_H__

#include <chrono>
#include <atomic>
#include <mutex>

template<typename TValue>
class ProgressMeter
{
  using steady_clock = std::chrono::steady_clock;

  std::ostream &sout;
  const std::string message;
  const float scale;
  TValue current;
  steady_clock::duration::rep printed;

  inline u32 CalcThousandths(TValue val) const
  {
    return (u32)(scale * val);
  }
  inline void PrintFraction(u32 fraction)
  {
    sout << message << fraction/10 << '.' << fraction%10 << "%\r" << std::flush;
    printed = steady_clock::now().time_since_epoch().count();
  }
  inline bool ShouldUpdate(TValue oldval, TValue newval, u32 &newfraction) const
  {
    newfraction = CalcThousandths(newval);
    if (CalcThousandths(oldval) == newfraction)
      return false;
    return steady_clock::now() - steady_clock::time_point(steady_clock::duration(printed)) >= std::chrono::milliseconds(50);
  }

public:
  ProgressMeter(std::ostream &sout, const std::string &message, TValue total) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0) {}
  ProgressMeter(std::ostream &sout, const char *message, TValue total) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0) {}

  // NOTE: Update() doesn't always update current value, so don't mix it with Add()
  void Update(TValue newval)
  {
    u32 newfraction;
    if (ShouldUpdate(current, newval, newfraction))
    {
      PrintFraction(newfraction);
      current = newval;
    }
  }
  void Add(TValue amount)
  {
    TValue oldval = current;
    current += amount;
    u32 newfraction;
    if (ShouldUpdate(oldval, current, newfraction))
      PrintFraction(newfraction);
  }
};

template<typename TValue>
class MTProgressMeter
{
  using steady_clock = std::chrono::steady_clock;

  std::ostream &sout;
  const std::string message;
  const float scale;
  std::atomic<TValue> current;
  std::atomic<steady_clock::duration::rep> printed;
  std::mutex &output_lock;

  inline u32 CalcThousandths(TValue val) const
  {
    return (u32)(scale * val);
  }
  inline void PrintFraction(u32 fraction)
  {
    output_lock.lock();
    sout << message << fraction/10 << '.' << fraction%10 << "%\r" << std::flush;
    output_lock.unlock();
    printed.store(steady_clock::now().time_since_epoch().count(), std::memory_order_relaxed);
  }
  inline bool ShouldUpdate(TValue oldval, TValue newval, u32 &newfraction) const
  {
    newfraction = CalcThousandths(newval);
    if (CalcThousandths(oldval) == newfraction)
      return false;
    steady_clock::duration::rep lastprinted = printed.load(std::memory_order_relaxed);
    return steady_clock::now() - steady_clock::time_point(steady_clock::duration(lastprinted)) >= std::chrono::milliseconds(50);
  }

public:
  MTProgressMeter(std::ostream &sout, const std::string &message, TValue total, std::mutex &output_lock) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0), output_lock(output_lock) {}
  MTProgressMeter(std::ostream &sout, const char *message, TValue total, std::mutex &output_lock) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0), output_lock(output_lock) {}

  void Add(TValue amount)
  {
    TValue oldval = current.fetch_add(amount, std::memory_order_relaxed);
    u32 newfraction;
    if (ShouldUpdate(oldval, oldval + amount, newfraction))
      PrintFraction(newfraction);
  }
  inline void AddSilent(TValue amount)
  {
    current.fetch_add(amount, std::memory_order_relaxed);
  }
  void ClearLine()
  {
    std::lock_guard<std::mutex> lock(output_lock);
    sout << std::setw(message.size()+6) << std::setfill(' ') << "\r";
  }
  void Print()
  {
    TValue val = current.load(std::memory_order_relaxed);
    PrintFraction(CalcThousandths(val));
  }
};


#endif // __PROGRESSMETER_H__
