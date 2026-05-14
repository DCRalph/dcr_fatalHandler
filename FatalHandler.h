#pragma once

#include <Arduino.h>
#include <functional>

namespace FatalHandler
{
  void install();
  [[noreturn]] void abortWithMessage(const char *context, const char *message);
  [[noreturn]] void abortCurrentException(const char *context);

  template <typename Func>
  void runGuardedTask(const char *taskName, Func &&func)
  {
    try
    {
      func();
    }
    catch (...)
    {
      abortCurrentException(taskName);
    }
  }
}
