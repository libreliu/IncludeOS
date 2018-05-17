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

//#define NDP_DEBUG 1
#ifdef NDP_DEBUG
#define PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINT(fmt, ...) /* fmt */
#endif
#include <net/ip6/ndp.hpp>
#include <net/inet>

namespace net
{
  int Ndp::receive_neighbor_solicitation(icmp6::Packet& req)
  {
    bool any_src = req.ip().ip_src() == IP6::ADDR_ANY;
    IP6::addr target = req.neighbor_sol().get_target();

    PRINT("ICMPv6 NDP Neighbor solicitation request\n");
    PRINT(">> target: %s\n", target.str().c_str());

    if (target.is_multicast()) {
        PRINT("ND: neighbor solictation target address is multicast\n");
        return -1;
    }

    icmp6::Packet res(inet_.ip6_packet_factory());

    // drop if the packet is too small
    if (res.ip().capacity() < IP6_HEADER_LEN
     + (int) res.header_size() + req.payload().size())
    {
      PRINT("WARNING: Network MTU too small for ICMP response, dropping\n");
      return -1;
    }

    // Populate response IP header
    res.ip().set_ip_src(inet_.ip6_addr());
    if (any_src) {
        res.ip().set_ip_dst(ip6::Addr::node_all_nodes);
    } else {
        res.ip().set_ip_dst(req.ip().ip_src());
    }
    res.ip().set_ip_hop_limit(255);

    // Populate response ICMP header
    res.set_type(ICMP_type::ND_NEIGHBOR_ADV);
    res.set_code(0);
    res.set_neighbor_adv_flag(NEIGH_ADV_SOL | NEIGH_ADV_OVERRIDE);
    PRINT("<ICMP6> Transmitting Neighbor adv to %s\n",
          res.ip().ip_dst().str().c_str());

    // Insert target link address, ICMP6 option header and our mac address
    // TODO: This is hacky. Fix it
    MAC::Addr dest_mac("c0:01:0a:00:00:2a");
    // Target link address
    res.set_payload({req.payload().data(), 16 });
    res.set_ndp_options_header(0x02, 0x01);
    res.set_payload({reinterpret_cast<uint8_t*> (&dest_mac), 6});

    // Add checksum
    res.set_checksum();

    PRINT("<ICMP6> Neighbor Adv Response size: %i payload size: %i, checksum: 0x%x\n",
          res.ip().size(), res.payload().size(), res.compute_checksum());

    network_layer_out_(res.release());
    return 0;
  }

  void Ndp::send_router_solicitation()
  {
    icmp6::Packet req(inet_.ip6_packet_factory());
    req.ip().set_ip_src(inet_.ip6_addr());
    req.ip().set_ip_dst(ip6::Addr::node_all_nodes);
    req.ip().set_ip_hop_limit(255);
    req.set_type(ICMP_type::ND_ROUTER_SOLICATION);
    req.set_code(0);
    req.set_reserved(0);
    // Set multicast addreqs
    // IPv6mcast_02: 33:33:00:00:00:02

    // Add checksum
    req.set_checksum();

    PRINT("<ICMP6> Router solicit size: %i payload size: %i, checksum: 0x%x\n",
          req.ip().size(), req.payload().size(), req.compute_checksum());

    network_layer_out_(req.release());
  }


  void Ndp::receive(icmp6::Packet& pckt) {
    switch(pckt.type()) {
    case (ICMP_type::ND_ROUTER_SOLICATION):
      PRINT("<ICMP6> ICMP Router solictation message from %s\n", pckt.ip().ip_src().str().c_str());
      break;
    case (ICMP_type::ND_ROUTER_ADV):
      PRINT("<ICMP6> ICMP Router advertisement message from %s\n", pckt.ip().ip_src().str().c_str());
      break;
    case (ICMP_type::ND_NEIGHBOR_SOL):
      PRINT("<ICMP6> ICMP Neigbor solictation message from %s\n", pckt.ip().ip_src().str().c_str());
      receive_neighbor_solicitation(pckt);
      break;
    case (ICMP_type::ND_NEIGHBOR_ADV):
      PRINT("<ICMP6> ICMP Neigbor advertisement message from %s\n", pckt.ip().ip_src().str().c_str());
      break;
    case (ICMP_type::ND_REDIRECT):
      PRINT("<ICMP6> ICMP Neigbor redirect message from %s\n", pckt.ip().ip_src().str().c_str());
      break;
    default:
      return;
    }
  }
}
