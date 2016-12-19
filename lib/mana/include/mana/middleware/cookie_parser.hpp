// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2016 Oslo and Akershus University College of Applied Sciences
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

#ifndef MANA_MIDDLEWARE_COOKIE_PARSER_HPP
#define MANA_MIDDLEWARE_COOKIE_PARSER_HPP

#include <net/http/cookie.hpp>
#include <mana/attributes/cookie_jar.hpp>
#include <mana/middleware.hpp>

namespace mana {
namespace middleware {

/**
 * @brief A way to parse cookies that the browser is sending to the server
 */
class CookieParser : public Middleware {
public:

  Callback handler() override {
    return {this, &CookieParser::process};
  }

  void process(Request_ptr req, Response_ptr res, Next next);

private:
  attribute::CookieJar req_cookies_;

  static const std::regex cookie_pattern_;

  bool has_cookie(mana::Request_ptr req) const noexcept;

  std::string read_cookies(mana::Request_ptr req) const noexcept;

  void parse(const std::string& cookie_data);

}; // < class CookieParser

/**--v----------- Implementation Details -----------v--**/

const std::regex CookieParser::cookie_pattern_ {"[^;]+"};

inline void CookieParser::process(mana::Request_ptr req, mana::Response_ptr, mana::Next next) {
  if(has_cookie(req)) {
    parse(read_cookies(req));
    auto jar_attr = std::make_shared<CookieJar>(req_cookies_);
    req->set_attribute(jar_attr);
  }

  return (*next)();
}

inline bool CookieParser::has_cookie(mana::Request_ptr req) const noexcept {
  return req->header().has_field(http::header::Cookie);
}

inline std::string CookieParser::read_cookies(mana::Request_ptr req) const noexcept {
  return req->header().value(http::header::Cookie).to_string();
}

void CookieParser::parse(const std::string& cookie_data) {
  if(cookie_data.empty()) {
    throw http::CookieException{"Cannot parse empty cookie-string!"};
  }

  req_cookies_.clear(); //< Clear {req_cookies_} for new entries

  auto position = std::sregex_iterator(cookie_data.begin(), cookie_data.end(), cookie_pattern_);
  auto end      = std::sregex_iterator();

  while (position not_eq end) {
    auto cookie = (*position++).str();

    cookie.erase(std::remove(cookie.begin(), cookie.end(), ' '), cookie.end());

    auto pos = cookie.find('=');
    if (pos not_eq std::string::npos) {
      req_cookies_.insert(cookie.substr(0, pos), cookie.substr(pos + 1));
    } else {
      req_cookies_.insert(cookie);
    }
  }

}

/**--^----------- Implementation Details -----------^--**/

}} //< namespace mana::middleware

#endif //< MIDDLEWARE_COOKIE_PARSER_HPP
