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
  const std::chrono::milliseconds PRINT_INTERVAL = std::chrono::milliseconds(50);

  std::ostream &sout;        // stream for output (for commandline, this is cout)
  const std::string message; // message to display alongside percentage
  const float scale;         // pre-computed multiplier to convert progress value into a percentage*10
  TValue current;            // last known progress value
  steady_clock::duration::rep printed; // last time progress was outputted

  inline u32 CalcThousandths(TValue val) const
  {
    return (u32)(scale * val + 0.5f);
  }
  inline bool PrintFraction(TValue oldval, TValue newval)
  {
    // if the displayed value won't change, don't print
    u32 newfraction = CalcThousandths(newval);
    if (CalcThousandths(oldval) == newfraction)
      return false;

    // check if enough time has passed
    steady_clock::time_point now = steady_clock::now();
    steady_clock::time_point lastpoint = steady_clock::time_point(steady_clock::duration(printed));

    // if enough time has passed, print the current progress, and update the time record
    if (now - lastpoint >= PRINT_INTERVAL || newfraction == 1000)
    {
      sout << message << newfraction/10 << '.' << newfraction%10 << "%\r" << std::flush;
      printed = now.time_since_epoch().count();
      return true;
    }
    return false;
  }

public:
  ProgressMeter(std::ostream &sout, const std::string &message, TValue total) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0) {}
  ProgressMeter(std::ostream &sout, const char *message, TValue total) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0) {}

  // NOTE: Update() doesn't always update current value, so don't mix it with Add()
  void Update(TValue newval)
  {
    if (PrintFraction(current, newval))
      current = newval;
  }
  void Add(TValue amount)
  {
    TValue oldval = current;
    current += amount;
    PrintFraction(oldval, current);
  }
};

template<typename TValue>
class MTProgressMeter
{
  using steady_clock = std::chrono::steady_clock;
  const std::chrono::milliseconds PRINT_INTERVAL = std::chrono::milliseconds(50);

  std::ostream &sout;         // stream for output (for commandline, this is cout)
  const std::string message;  // message to display alongside percentage
  const float scale;          // pre-computed multiplier to convert progress value into a percentage*10
  std::atomic<TValue> current; // last known progress value
  std::atomic<steady_clock::duration::rep> printed; // last time progress was outputted
  std::mutex &output_lock;

  inline u32 CalcThousandths(TValue val) const
  {
    return (u32)(scale * val + 0.5f);
  }
  inline bool PrintFraction(TValue oldval, TValue newval)
  {
    // if the displayed value won't change, don't print
    u32 newfraction = CalcThousandths(newval);
    if (CalcThousandths(oldval) == newfraction)
      return false;

    // check if enough time has passed
    steady_clock::time_point now = steady_clock::now();
    steady_clock::time_point lastpoint = steady_clock::time_point(steady_clock::duration(printed.load(std::memory_order_relaxed)));

    // if enough time has passed, print the current progress, and update the time record
    if (now - lastpoint >= PRINT_INTERVAL || newfraction == 1000)
    {
      output_lock.lock();
      sout << message << newfraction/10 << '.' << newfraction%10 << "%\r" << std::flush;
      output_lock.unlock();
      printed.store(now.time_since_epoch().count(), std::memory_order_relaxed);
      return true;
    }
    return false;
  }

public:
  MTProgressMeter(std::ostream &sout, const std::string &message, TValue total, std::mutex &output_lock) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0), output_lock(output_lock) {}
  MTProgressMeter(std::ostream &sout, const char *message, TValue total, std::mutex &output_lock) :
    sout(sout), message(message), scale(1000.0f / total), current(0), printed(0), output_lock(output_lock) {}

  void Add(TValue amount)
  {
    TValue oldval = current.fetch_add(amount, std::memory_order_relaxed);
    PrintFraction(oldval, oldval + amount);
  }
  // print a line whilst progress is still running
  void PrintLine(const std::string &line)
  {
    TValue val = current.load(std::memory_order_relaxed);
    u32 fraction = CalcThousandths(val);
    std::lock_guard<std::mutex> lock(output_lock);
    sout << std::setw(message.size()+7) << std::setfill(' ') << "\r"
      << line << '\n'
      << message << fraction/10 << '.' << fraction%10 << "%\r" << std::flush;
  }
};


#endif // __PROGRESSMETER_H__
