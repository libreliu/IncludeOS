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

#include <net/dns/client.hpp>

namespace net
{
  Timer::duration_t DNSClient::DEFAULT_RESOLVE_TIMEOUT{std::chrono::seconds(5)};

  void DNSClient::resolve(IP4::addr dns_server, const std::string& hostname, Stack::resolve_func<IP4> func)
  {
    // create DNS request
    DNS::Request request;
    std::array<char, 256> buf{};
    size_t len  = request.create(buf.data(), hostname);

    auto key = request.get_id();

    // store the request for later match
    requests_.emplace(std::piecewise_construct,
      std::forward_as_tuple(key),
      std::forward_as_tuple(std::move(request), std::move(func)));

    // send request to DNS server
    socket_.sendto(dns_server, DNS::DNS_SERVICE_PORT, buf.data(), len, [this, dns_server, key] (Error err) {
      // If an error is not received, this will never execute (Error is just erased from the map
      // without calling the callback)

      INFO("DNS", "Couldn't resolve DNS server at %s. Reason: %s", dns_server.to_string().c_str(), err.what());

      // Find the request and remove it since an error occurred
      auto it = requests_.find(key);
      if (it != requests_.end()) {
        it->second.callback(IP4::ADDR_ANY, err);
        requests_.erase(it);
      }
    });
  }

  void DNSClient::receive_response(IP4::addr, UDP::port_t, const char* data, size_t)
  {
    const auto& reply = *(DNS::header*) data;
    // match the transactions id on the reply with the ones in our map
    auto it = requests_.find(ntohs(reply.id));
    // if this is match
    if(it != requests_.end())
    {
      // TODO: do some necessary validation ... (truncate etc?)

      // parse request
      it->second.request.parseResponse(data);
      // fire onResolve event
      it->second.finish();

      // the request is finished, removed it from our map
      requests_.erase(it);
    }
    else
    {
      debug("<DNSClient::receive_response> Cannot find matching DNS Request with transid=%u\n", ntohs(reply.id));
    }

  }
}
