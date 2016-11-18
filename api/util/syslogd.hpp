// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Weak default printf
// Plugin UDP

#pragma once
#ifndef UTIL_SYSLOGD_HPP
#define UTIL_SYSLOGD_HPP

#include <cstdio>
#include <string>
#include <regex>

#include <syslog.h>             // POSIX symbolic constants
#include "syslog_facility.hpp"

const int BUFLEN = 2048;
const int TIMELEN = 32;

class Syslog {

public:
  // Parameter pack arguments
  template <typename... Args>
  static void syslog(int priority, const char* message, Args&&... args) {
    // snprintf removes % if calling syslog with %m in addition to arguments
    // Find %m here first and escape % if found
    std::regex m_regex{"\\%m"};
    std::string msg = std::regex_replace(message, m_regex, "%%m");

    char buf[BUFLEN];
    snprintf(buf, BUFLEN, msg.c_str(), args...);
    syslog(priority, buf);
  }

  // va_list arguments (POSIX)
  static void syslog(int priority, const char* message, va_list args);

  __attribute__((weak))
  static void syslog(int priority, const char* buf);

  __attribute__((weak))
  static void openlog(const char* ident, int logopt, int facility);

  __attribute__((weak))
  static void closelog();

  static bool valid_priority(int priority) noexcept {
    return not ((priority < LOG_EMERG) or (priority > LOG_DEBUG));
  }

  static bool valid_logopt(int logopt) noexcept {
    return (logopt & LOG_PID or logopt & LOG_CONS or logopt & LOG_NDELAY or
      logopt & LOG_ODELAY or logopt & LOG_NOWAIT or logopt & LOG_PERROR);
  }

  static bool valid_facility(int facility) noexcept {
    return ( (facility >= LOG_KERN and facility <= LOG_CRON ) or (facility >= LOG_LOCAL0 and facility <= LOG_LOCAL7) );
  }

private:
  static std::unique_ptr<Syslog_facility> fac_;
};

#endif
