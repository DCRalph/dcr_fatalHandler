#include "fatalHandler/FatalHandler.h"

#include <exception>

#include <esp_system.h>
#include <logger/Logger.h>

#undef LOG_TAG
#define LOG_TAG "FATAL"

namespace
{
  void writeFatalRaw(const char *context, const char *message)
  {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[FATL] [%s] %s\n",
             context != nullptr ? context : "FATAL",
             message != nullptr ? message : "Unknown fatal error");
    LoggerInternal::Raw(buffer);
  }

  [[noreturn]] void abortNow()
  {
    LoggerInternal::Flush();
    delay(50);
    abort();
  }

  void printActiveException(const char *context)
  {
    try
    {
      throw;
    }
    catch (const std::exception &ex)
    {
      LoggerInternal::LogText(LogLevel::Fatal, context, ex.what());
      writeFatalRaw(context, ex.what());
    }
    catch (...)
    {
      LoggerInternal::LogText(LogLevel::Fatal, context, "Unknown non-std exception");
      writeFatalRaw(context, "Unknown non-std exception");
    }
  }

  [[noreturn]] void terminateHandler()
  {
    LoggerInternal::Raw("[FATL] [TERMINATE] std::terminate invoked\n");
    try
    {
      std::exception_ptr eptr = std::current_exception();
      if (eptr)
      {
        std::rethrow_exception(eptr);
      }
      else
      {
        LoggerInternal::LogText(LogLevel::Fatal, "TERMINATE", "No active exception");
      }
    }
    catch (...)
    {
      printActiveException("TERMINATE");
    }

    abortNow();
  }
}

void FatalHandler::install()
{
  std::set_terminate(terminateHandler);
  debugI("Terminate handler installed");
}

[[noreturn]] void FatalHandler::abortWithMessage(const char *context, const char *message)
{
  LoggerInternal::LogText(LogLevel::Fatal, context, message);
  writeFatalRaw(context, message);
  abortNow();
}

[[noreturn]] void FatalHandler::abortCurrentException(const char *context)
{
  writeFatalRaw(context, "Unhandled exception in task");
  printActiveException(context);
  abortNow();
}
