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

#pragma once
#ifndef NET_IP6_NDP_HPP
#define NET_IP6_NDP_HPP

#include <rtc>
#include <cmath>
#include <unordered_map>
#include <deque>
#include <util/timer.hpp>
#include "packet_icmp6.hpp"
#include "packet_ndp.hpp"

using namespace std::chrono_literals;
namespace net {

  class ICMPv6;

  /** NDP manager, including an NDP-Cache. */
  class Ndp {

  public:
    // Router constants
    static const int        MAX_INITIAL_RTR_ADVERT_INTERVAL = 16;  // in seconds
    static const int        MAX_INITIAL_RTR_ADVERTISEMENTS  = 3;   // transmissions
    static const int        MAX_FINAL_RTR_ADVERTISEMENTS    = 3;   // transmissions
    static constexpr double MIN_DELAY_BETWEEN_RAS           = 3;   // in seconds
    static constexpr double MAX_RA_DELAY_TIME               = 0.5; // in seconds

    // Host constants
    static const int MAX_RTR_SOLICITATION_DELAY = 1; // in seconds
    static const int RTR_SOLICITATION_INTERVAL  = 4; // in seconds
    static const int MAX_RTR_SOLICITATIONS      = 3; // transmissions

    // Node constants
    static const int        MAX_MULTICAST_SOLICIT      = 3;     // transmissions
    static const int        MAX_UNICAST_SOLICIT        = 3;     // transmissions
    static const int        MAX_ANYCAST_DELAY_TIME     = 1;     // in seconds
    static const int        MAX_NEIGHBOR_ADVERTISEMENT = 3;     // transmissions
    static const int        REACHABLE_TIME             = 30000; // in milliseconds
    static const int        RETRANS_TIMER              = 1000;  // in milliseconds
    static const int        DELAY_FIRST_PROBE_TIME     = 5;     // in seconds
    static constexpr double MIN_RANDOM_FACTOR          = 0.5;
    static constexpr double MAX_RANDOM_FACTOR          = 1.5;

    // Neighbour flag constants
    static const uint32_t NEIGH_UPDATE_OVERRIDE          = 0x00000001;
    static const uint32_t NEIGH_UPDATE_WEAK_OVERRIDE     = 0x00000002;
    static const uint32_t NEIGH_UPDATE_OVERRIDE_ISROUTER = 0x00000004;
    static const uint32_t NEIGH_UPDATE_ISROUTER          = 0x40000000;
    static const uint32_t NEIGH_UPDATE_ADMIN             = 0x80000000;

    enum class NeighbourStates : uint8_t {
      INCOMPLETE,
      REACHABLE,
      STALE,
      DELAY,
      PROBE,
      FAIL
    };
    using Stack   = IP6::Stack;
    using Route_checker = delegate<bool(ip6::Addr)>;
    using Ndp_resolver = delegate<void(ip6::Addr)>;
    using Dad_handler = delegate<void()>;
    using RouterAdv_handler = delegate<void(ip6::Addr)>;
    using ICMP_type = ICMP6_error::ICMP_type;

    /** Constructor */
    explicit Ndp(Stack&) noexcept;

    /** Handle incoming NDP packet. */
    void receive(icmp6::Packet& pckt);
    void receive_neighbour_solicitation(icmp6::Packet& pckt);
    void receive_neighbour_advertisement(icmp6::Packet& pckt);
    void receive_router_solicitation(icmp6::Packet& pckt);
    void receive_router_advertisement(icmp6::Packet& pckt);
    void receive_redirect(icmp6::Packet& req);

    /** Send out NDP packet */
    void send_neighbour_solicitation(ip6::Addr target);
    void send_neighbour_advertisement(icmp6::Packet& req);
    void send_router_solicitation();
    void send_router_solicitation(RouterAdv_handler delg);
    void send_router_advertisement();

    /** Roll your own ndp-resolution system. */
    void set_resolver(Ndp_resolver ar)
    { ndp_resolver_ = ar; }

    enum Resolver_name { DEFAULT, HH_MAP };

    /**
     * Set NDP proxy policy.
     * No route checker (default) implies NDP proxy functionality is disabled.
     *
     * @param delg : delegate to determine if we should reply to a given IP
     */
    void set_proxy_policy(Route_checker delg)
    { proxy_ = delg; }

    void perform_dad(ip6::Addr, Dad_handler delg);
    void dad_completed();
    void add_addr_autoconf(ip6::Addr ip, uint32_t preferred_lifetime,
            uint32_t valid_lifetime);
    void add_addr_onlink(ip6::Addr ip, uint32_t valid_lifetime);
    void add_addr_static(ip6::Addr ip, uint32_t valid_lifetime);
    void add_router(ip6::Addr ip, uint16_t router_lifetime);

    /** Downstream transmission. */
    void transmit(Packet_ptr, ip6::Addr next_hop, MAC::Addr mac = MAC::EMPTY);

    /** Cache IP resolution. */
    void cache(ip6::Addr ip, MAC::Addr mac, NeighbourStates state, uint32_t flags, bool update=true);
    void cache(ip6::Addr ip, uint8_t *ll_addr, NeighbourStates state, uint32_t flags, bool update=true);

    /* Destination cache */
    void dest_cache(ip6::Addr dest_ip, ip6::Addr next_hop);
    void delete_dest_entry(ip6::Addr ip);

    /** Lookup for cache entry */
    bool lookup(ip6::Addr ip);

    /* Check for Neighbour Reachabilty periodically */
    void check_neighbour_reachability();

    /** Flush the NDP cache */
    void flush_cache()
    { neighbour_cache_.clear(); };

    /** Flush expired entries */
    void flush_expired_neighbours();
    void flush_expired_routers();
    void flush_expired_prefix();

    void set_neighbour_cache_flush_interval(std::chrono::minutes m) {
      flush_interval_ = m;
    }

    // Delegate output to link layer
    void set_linklayer_out(downstream_link s)
    { linklayer_out_ = s; }

    MAC::Addr& link_mac_addr()
    { return mac_; }

    const ip6::Addr& static_ip() const noexcept
    { return ip6_addr_; }

    uint8_t static_prefix() const noexcept
    { return ip6_prefix_; }

    ip6::Addr static_gateway() const noexcept
    { return ip6_gateway_; }

    void set_static_addr(ip6::Addr addr)
    { ip6_addr_ = std::move(addr); }

    void set_static_gateway(ip6::Addr addr)
    { ip6_gateway_ = addr; }

    void set_static_prefix(uint8_t prefix)
    { ip6_prefix_ = prefix; }

  private:

    /** NDP cache expires after neighbour_cache_exp_sec_ seconds */
    static constexpr uint16_t neighbour_cache_exp_sec_ {60 * 5};

    /** Cache entries are just MAC's and timestamps */
    struct Neighbour_Cache_entry {
      /** Map needs empty constructor (we have no emplace yet) */
      Neighbour_Cache_entry() noexcept = default;

      Neighbour_Cache_entry(MAC::Addr mac, NeighbourStates state, uint32_t flags) noexcept
      : mac_(mac), timestamp_(RTC::time_since_boot()), flags_(flags) {
          set_state(state);
      }

      Neighbour_Cache_entry(const Neighbour_Cache_entry& cpy) noexcept
      : mac_(cpy.mac_), state_(cpy.state_),
        timestamp_(cpy.timestamp_), flags_(cpy.flags_) {}

      void update() noexcept { timestamp_ = RTC::time_since_boot(); }

      bool expired() const noexcept
      { return RTC::time_since_boot() > timestamp_ + neighbour_cache_exp_sec_; }

      MAC::Addr mac() const noexcept
      { return mac_; }

      RTC::timestamp_t timestamp() const noexcept
      { return timestamp_; }

      RTC::timestamp_t expires() const noexcept
      { return timestamp_ + neighbour_cache_exp_sec_; }

      void set_state(NeighbourStates state)
      {
         if (state <= NeighbourStates::FAIL) {
           state_ = state;
         }
      }

      void set_flags(uint32_t flags)
      {
          flags_ = flags;
      }

      std::string get_state_name()
      {
          switch(state_) {
          case NeighbourStates::INCOMPLETE:
              return "incomplete";
          case NeighbourStates::REACHABLE:
              return "reachable";
          case NeighbourStates::STALE:
              return "stale";
          case  NeighbourStates::DELAY:
              return "delay";
          case NeighbourStates::PROBE:
              return "probe";
          case NeighbourStates::FAIL:
              return "fail";
          default:
              return "uknown";
          }
      }

    private:
      MAC::Addr        mac_;
      NeighbourStates  state_;
      RTC::timestamp_t timestamp_;
      uint32_t         flags_;
    }; //< struct Neighbour_Cache_entry

    struct Destination_Cache_entry {
      Destination_Cache_entry(ip6::Addr next_hop)
        : next_hop_{next_hop} {}

    void update(ip6::Addr next_hop)
    {
      next_hop_ = next_hop;
    }

    ip6::Addr next_hop()
    { return next_hop_; }

    private:
      ip6::Addr        next_hop_;
      // TODO: Add PMTU and Round-trip timers
    };

    struct Queue_entry {
      Queue_entry(Packet_ptr p)
        : pckt{std::move(p)}
      {}
      Packet_ptr pckt;
      int tries_remaining = MAX_MULTICAST_SOLICIT;
    };

    struct Prefix_entry {
    public:
      Prefix_entry(ip6::Addr prefix, uint32_t preferred_lifetime,
            uint32_t valid_lifetime)
          : prefix_{prefix}
      {
        preferred_ts_ = preferred_lifetime ?
            (RTC::time_since_boot() + preferred_lifetime) : 0;
        valid_ts_ = valid_lifetime ?
            (RTC::time_since_boot() + valid_lifetime) : 0;
      }

      ip6::Addr prefix() const noexcept
      { return prefix_; }

      bool preferred() const noexcept
      { return preferred_ts_ ? RTC::time_since_boot() < preferred_ts_ : true; }

      bool valid() const noexcept
      { return valid_ts_ ? RTC::time_since_boot() > valid_ts_ : true; }

      bool always_valid() const noexcept
      { return valid_ts_ ? false : true; }

      uint32_t remaining_valid_time()
      { return valid_ts_ < RTC::time_since_boot() ? 0 : valid_ts_ - RTC::time_since_boot(); }

      void update_preferred_lifetime(uint32_t preferred_lifetime)
      {
        preferred_ts_ = preferred_lifetime ?
            (RTC::time_since_boot() + preferred_lifetime) : 0;
      }

      void update_valid_lifetime(uint32_t valid_lifetime)
      {
        valid_ts_ = valid_lifetime ?
          (RTC::time_since_boot() + valid_lifetime) : 0;
      }
    private:
      ip6::Addr        prefix_;
      RTC::timestamp_t preferred_ts_;
      RTC::timestamp_t valid_ts_;
    };

    struct Router_entry {
      Router_entry(ip6::Addr router, uint16_t lifetime) :
        router_{router}
      {
        update_router_lifetime(lifetime);
      }

      ip6::Addr router() const noexcept
      { return router_; }

      bool expired() const noexcept
      { return RTC::time_since_boot() > invalidation_ts_; }

      void update_router_lifetime(uint16_t lifetime)
      { invalidation_ts_ = RTC::time_since_boot() + lifetime; }

    private:
      ip6::Addr        router_;
      RTC::timestamp_t invalidation_ts_;
    };

    using Cache       = std::unordered_map<ip6::Addr, Neighbour_Cache_entry>;
    using DestCache   = std::unordered_map<ip6::Addr, Destination_Cache_entry>;
    using PacketQueue = std::unordered_map<ip6::Addr, Queue_entry>;
    using PrefixList  = std::deque<Prefix_entry>;
    using RouterList  = std::deque<Router_entry>;

    // Ndp host parameters configured for a particular inet stack
    struct HostNdpParameters {
    public:
      HostNdpParameters() :
        link_mtu_{1500}, cur_hop_limit_{255},
        base_reachable_time_{REACHABLE_TIME},
        retrans_time_{RETRANS_TIMER} {
          reachable_time_ = compute_reachable_time();
        }

      // Compute random time in the range of min and max
      // random factor times base reachable time
      double compute_reachable_time()
      {
        auto lower = MIN_RANDOM_FACTOR * base_reachable_time_;
        auto upper = MAX_RANDOM_FACTOR * base_reachable_time_;

        return (std::fmod(rand(), (upper - lower + 1)) + lower);
      }

      uint16_t link_mtu_;
      uint8_t  cur_hop_limit_;
      uint32_t base_reachable_time_;
      uint32_t reachable_time_;
      uint32_t retrans_time_;
    };

    // Ndp router parameters configured for a particular inet stack
    struct RouterNdpParameters {
    public:
      RouterNdpParameters() :
        is_router_{false}, send_advertisements_{false},
        managed_flag_{false}, other_flag_{false},
        cur_hop_limit_{255}, link_mtu_{0},
        max_ra_interval_{600}, min_ra_interval_{max_ra_interval_},
        default_lifetime_(3 * max_ra_interval_), reachable_time_{0},
        retrans_time_{0} {}

    private:
      bool       is_router_;
      bool       send_advertisements_;
      bool       managed_flag_;
      bool       other_flag_;
      uint8_t    cur_hop_limit_;
      uint16_t   link_mtu_;
      uint16_t   max_ra_interval_;
      uint16_t   min_ra_interval_;
      uint16_t   default_lifetime_;
      uint32_t   reachable_time_;
      uint32_t   retrans_time_;
      PrefixList prefix_list_;
    };

    /** Stats */
    uint32_t& requests_rx_;
    uint32_t& requests_tx_;
    uint32_t& replies_rx_;
    uint32_t& replies_tx_;

    std::chrono::minutes flush_interval_ = 5min;

    Timer neighbour_reachability_timer_ {{ *this, &Ndp::check_neighbour_reachability }};
    Timer resolve_timer_ {{ *this, &Ndp::resolve_waiting }};
    Timer flush_neighbour_timer_ {{ *this, &Ndp::flush_expired_neighbours }};
    Timer flush_prefix_timer_ {{ *this, &Ndp::flush_expired_prefix }};
    Timer flush_router_timer_ {{ *this, &Ndp::flush_expired_routers }};

    Stack& inet_;
    Route_checker       proxy_ = nullptr;
    Dad_handler         dad_handler_ = nullptr;
    RouterAdv_handler   ra_handler_ = nullptr;
    HostNdpParameters   host_params_;
    RouterNdpParameters router_params_;

    /* Static IP6 configuration for inet.
     * Dynamic ip6 addresses are present in prefix list.
     * We could have this in ip6 instead but gets confusing since ip6
     * stack can multiple ip6 addresses. */
    ip6::Addr ip6_addr_;
    ip6::Addr ip6_gateway_;
    uint8_t   ip6_prefix_;

    MAC::Addr mac_;
    ip6::Addr tentative_addr_ = IP6::ADDR_ANY;

    // Outbound data goes through here */
    downstream_link linklayer_out_ = nullptr;

    // The caches
    Cache     neighbour_cache_ {};
    DestCache dest_cache_ {};

    // Packet queue
    PacketQueue waiting_packets_;

    // Prefix List
    PrefixList prefix_list_;
    RouterList router_list_;

    // Settable resolver - defualts to ndp_resolve
    Ndp_resolver ndp_resolver_ = {this, &Ndp::ndp_resolve};

    /** Send an ndp resolution request */
    void ndp_resolve(ip6::Addr next_hop);

    /**
     * Add a packet to waiting queue, to be sent when IP is resolved.
     */
    void await_resolution(Packet_ptr, ip6::Addr);

    /** Create a default initialized NDP-packet */
    Packet_ptr create_packet();

    /** Retry ndp-resolution for packets still waiting */
    void resolve_waiting();

    HostNdpParameters& host()
    { return host_params_; }

    RouterNdpParameters& router()
    { return router_params_; }
  }; //< class Ndp

} //< namespace net

#endif //< NET_NDP_HPP
