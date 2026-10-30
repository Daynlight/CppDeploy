// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <queue>
#include <mutex>
#include <string>
#include <string_view>
#include <chrono>

#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

#include "../config.h"



namespace CD {
struct LSD {
  const char* tag;
  fmt::text_style style;
};

namespace LSC {
  inline constexpr fmt::text_style Main =   fg(fmt::color::deep_pink);
  inline constexpr fmt::text_style Server = fg(fmt::color::chocolate);
  inline constexpr fmt::text_style Error =  fg(fmt::color::dark_red);
};

namespace LST {
  inline constexpr const char* SignalHandler        = "Signal Handler";
  inline constexpr const char* Main                 = "Main";
  inline constexpr const char* Server               = "Server";
  inline constexpr const char* ServerThread         = "Server Thread";
  inline constexpr const char* ServerClientHandler  = "Server Client Handler";
};



namespace LS {
  inline constexpr LSD SginalBold               { CD::LST::SignalHandler,       CD::LSC::Main | fmt::emphasis::bold   };
  inline constexpr LSD MainBold                 { CD::LST::Main,                CD::LSC::Main | fmt::emphasis::bold   };
  inline constexpr LSD Main                     { CD::LST::Main,                CD::LSC::Main                         };
  inline constexpr LSD MainError                { CD::LST::Main,                CD::LSC::Error                        };
  inline constexpr LSD ServerBold               { CD::LST::Server,              CD::LSC::Server | fmt::emphasis::bold };
  inline constexpr LSD Server                   { CD::LST::Server,              CD::LSC::Server                       };
  inline constexpr LSD ServerError              { CD::LST::Server,              CD::LSC::Error                        };
  inline constexpr LSD ServerThread             { CD::LST::ServerThread,        CD::LSC::Server                       };
  inline constexpr LSD ServerThreadError        { CD::LST::ServerThread,        CD::LSC::Error                        };
  inline constexpr LSD ServerClientHandler      { CD::LST::ServerClientHandler, CD::LSC::Server                       };
  inline constexpr LSD ServerClientHandlerError { CD::LST::ServerClientHandler, CD::LSC::Error                        };
};



class Logger {
// =========================== //
// === Data ================== //
// =========================== //
private:
  std::mutex log_mutex;

// =========================== //
// === Functional ============ //
// =========================== //
// === Constructors
private:
  Logger();
  ~Logger() = default;
  Logger(const Logger& second) = delete;
  Logger& operator=(const Logger& second) = delete;
  Logger(Logger&& second) = delete;
  Logger& operator=(Logger&& second) = delete;

// === Singleton
public:
  static Logger& get();

// === Log
public:
  template <typename... T>
  void print(LSD type, std::string_view fmt_str, T&&... args);
};
};



#include "logger.hpp"

