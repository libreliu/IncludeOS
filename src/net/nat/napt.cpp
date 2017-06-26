// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2017 Oslo and Akershus University College of Applied Sciences
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

#include <net/nat/napt.hpp>

namespace net {
namespace nat {

IP4::IP_packet_ptr NAPT::nat(IP4::IP_packet_ptr pkt, const Stack& inet)
{
  if(pkt->ip_dst() != inet.ip_addr())
  {
    pkt = snat(std::move(pkt), inet);
  }
  else
  {
    pkt = dnat(std::move(pkt), inet);
  }

  return pkt;
}

// Replace source socket with external (this) address and random port
IP4::IP_packet_ptr NAPT::snat(IP4::IP_packet_ptr pkt, const Stack& inet)
{
  // Early return if the packet is for me
  if(UNLIKELY(pkt->ip_dst() == inet.ip_addr()))
    return pkt;

  if(pkt->ip_protocol() == Protocol::TCP)
  {
    snat(*static_cast<tcp::Packet*>(pkt.get()), inet.ip_addr());
  }

  return pkt;
}

// Replace source address with external address from table
IP4::IP_packet_ptr NAPT::dnat(IP4::IP_packet_ptr pkt, const Stack& inet)
{
  // Early return if the packet isn't for me
  if(UNLIKELY(pkt->ip_dst() != inet.ip_addr()))
    return pkt;

  if(pkt->ip_protocol() == Protocol::TCP)
  {
    dnat(*static_cast<tcp::Packet*>(pkt.get()));
  }

  return pkt;
}

void NAPT::snat(tcp::Packet& pkt, ip4::Addr src_ip)
{
  // Get the Socket
  Socket socket = pkt.source();

  // Is there an entry?
  auto it = std::find_if(tcp_trans.begin(), tcp_trans.end(),
    [socket] (auto& ent) {
      return ent.second == socket;
    });

  // If there already is an entry
  if(it != tcp_trans.end())
  {
    // Replace the source port with the already translated one
    pkt.set_src_port(it->first);
  }
  // If not
  else
  {
    // Generate a new port
    auto port = tcp_ports.get_next_ephemeral();

    // Replace the source port
    pkt.set_src_port(port);

    add_entry(port, socket);
  }

  // At last, replace the source address
  pkt.set_ip_src(src_ip);

  printf("SNAT %s => %s\n", socket.to_string().c_str(), pkt.source().to_string().c_str());

  // Recalculate checksum
  recalculate_checksum(pkt);
}

void NAPT::dnat(tcp::Packet& pkt)
{
  auto dst_port = pkt.dst_port();

  // Is there an entry?
  auto it = tcp_trans.find(dst_port);

  // If there already is an entry
  if(it != tcp_trans.end())
  {
    // Get the Socket
    auto socket = it->second;
    printf("DNAT %s => %s\n", pkt.destination().to_string().c_str(), socket.to_string().c_str());
    // Replace the destination port with the original one
    pkt.set_destination(socket);

    // Recalculate checksum
    recalculate_checksum(pkt);
  }
}

}
}
