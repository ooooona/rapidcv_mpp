#include "rapidcv_mpp.h"

namespace rapidcv_mpp {

#define NONE "\e[0m"
#define BLACK "\e[0;30m"
#define L_BLACK "\e[1;30m"
#define RED "\e[0;31m"
#define L_RED "\e[1;31m"
#define GREEN "\e[0;32m"
#define L_GREEN "\e[1;32m"
#define BROWN "\e[0;33m"
#define YELLOW "\e[1;33m"
#define BLUE "\e[0;34m"
#define L_BLUE "\e[1;34m"
#define PURPLE "\e[0;35m"
#define L_PURPLE "\e[1;35m"
#define CYAN "\e[0;36m"
#define L_CYAN "\e[1;36m"
#define GRAY "\e[0;37m"
#define WHITE "\e[1;37m"

#define BOLD "\e[1m"
#define UNDERLINE "\e[4m"
#define BLINK "\e[5m"
#define REVERSE "\e[7m"
#define HIDE "\e[8m"
#define CLEAR "\e[2J"
#define CLRLINE "\r\e[K"  // or "\e[1K\r"

static LogLevel ll = LogLevel::DEBUG;

static LogLevel getLogLevel() { return ll; }

#define ERROR(fmt, args...)                                   \
  if (getLogLevel() >= LogLevel::ERROR) {                     \
    do {                                                      \
      printf(RED "<ERROR> <%s:%d> ", __FUNCTION__, __LINE__); \
      printf(fmt, ##args);                                    \
      printf(NONE "\n");                                      \
    } while (0);                                              \
  }

#define WARN(fmt, args...)                                      \
  if (getLogLevel() >= LogLevel::WARN) {                        \
    do {                                                        \
      printf(YELLOW "<WARN> <%s:%d> ", __FUNCTION__, __LINE__); \
      printf(fmt, ##args);                                      \
      printf(NONE "\n");                                        \
    } while (0);                                                \
  }

#define INFO(fmt, args...)                                     \
  if (getLogLevel() >= LogLevel::INFO) {                       \
    do {                                                       \
      printf(GREEN "<INFO> <%s:%d> ", __FUNCTION__, __LINE__); \
      printf(fmt, ##args);                                     \
      printf(NONE "\n");                                       \
    } while (0);                                               \
  }

#define DEBUG(fmt, args...)                                    \
  if (getLogLevel() >= LogLevel::DEBUG) {                      \
    do {                                                       \
      printf(NONE "<DEBUG> <%s:%d> ", __FUNCTION__, __LINE__); \
      printf(fmt, ##args);                                     \
      printf(NONE "\n");                                       \
    } while (0);                                               \
  }

}  // namespace rapidcv_mpp
