#ifndef __FOREACH_PARALLEL_H__
#define __FOREACH_PARALLEL_H__

#include <functional>
#include <thread>
#include <atomic>
#include <future>

template <class T>
void foreach_parallel(const std::vector<T> &collection, u32 numThreads, const std::function<void(const T&)> &fn)
{
  if (numThreads == 1 || collection.size() == 1)
  {
    for (const auto &item : collection)
      fn(item);
  }
  else if (numThreads >= collection.size()) 
  {
    // spawn a thread for each item
    std::vector<std::future<void>> tasks;
    tasks.reserve(collection.size());
    for (const auto &item : collection)
      tasks.push_back(std::async(std::launch::async, std::bind(fn, std::ref(item))));
    for (auto &task : tasks)
      task.wait();
  }
  else
  {
    std::atomic<unsigned> itemPos(0);
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (unsigned thread = 0; thread < numThreads; thread++)
    {
      threads.emplace_back([&itemPos, &collection, &fn]()
      {
        while (1)
        {
          unsigned i = itemPos.fetch_add(1, std::memory_order_relaxed);
          if (i >= collection.size()) break;
          fn(collection.at(i));
        }
      });
    }
    for (auto &thread : threads)
      thread.join();
  }
}

#endif // __FOREACH_PARALLEL_H__
