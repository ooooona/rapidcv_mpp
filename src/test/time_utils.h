#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H

#include <sys/time.h>

#include <chrono>

namespace yt_utils {
template <typename Clock = std::chrono::high_resolution_clock>
class basic_timer {
 public:
  // clang-format off
  using timepoint_t = typename std::chrono::time_point<Clock, typename Clock::duration>;
  using ns = std::chrono::nanoseconds;
  using us = std::chrono::microseconds;
  using ms = std::chrono::milliseconds;
  using  s = std::chrono::seconds;
  using  m = std::chrono::minutes;
  using  h = std::chrono::hours;
  // clang-format on

 public:
  void tick() { this->m_tick = Clock::now(); }

  void tick(const timepoint_t &tp) { this->m_tick = tp; }

  void tock() { this->m_tock = Clock::now(); }

  void tock(const timepoint_t &tp) { this->m_tock = tp; }

  template <typename DurationType>
  int elapse() const {
    using namespace std::chrono;
    return this->elapse_dispatch<DurationType>();
  }

#define DEFINE_ELAPSE_HELPER(DURATION_TYPE) \
  int elapse_##DURATION_TYPE() const { return this->elapse_dispatch<DURATION_TYPE>(); }

  DEFINE_ELAPSE_HELPER(ns)
  DEFINE_ELAPSE_HELPER(us)
  DEFINE_ELAPSE_HELPER(ms)
  DEFINE_ELAPSE_HELPER(s)
  DEFINE_ELAPSE_HELPER(m)
  DEFINE_ELAPSE_HELPER(h)

 private:  // methods
  template <typename DurationType>
  int elapse_dispatch() const {
    using namespace std::chrono;
    return duration_cast<DurationType>(m_tock - m_tick).count();
  }

 private:
  timepoint_t m_tick;
  timepoint_t m_tock;
};

}  // namespace yt_utils

#endif
