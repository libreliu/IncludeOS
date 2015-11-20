#define DEBUG
#include <net/dhcp/dh4client.hpp>
#include <net/dhcp/dhcp4.hpp>
#include <debug>

// BOOTP (rfc951) message types
#define BOOTREQUEST 1
#define BOOTREPLY   2
 
// Possible values for flags field
#define BOOTP_UNICAST   0x0000
#define BOOTP_BROADCAST 0x8000
 
// Possible values for hardware type (htype) field
#define HTYPE_ETHER     1  // Ethernet 10Mbps
#define HTYPE_IEEE802   6  // IEEE 802.2 Token Ring
#define HTYPE_FDDI      8  // FDDI
 
/* Magic cookie validating dhcp options field (and bootp vendor
   extensions field). */
#define DHCP_OPTIONS_COOKIE "\143\202\123\143"
 
// DHCP Option codes
#define DHO_PAD                  0
#define DHO_SUBNET_MASK          1
#define DHO_TIME_OFFSET          2
#define DHO_ROUTERS              3
#define DHO_TIME_SERVERS         4
#define DHO_NAME_SERVERS         5
#define DHO_DOMAIN_NAME_SERVERS  6
#define DHO_LOG_SERVERS          7
#define DHO_COOKIE_SERVERS       8
#define DHO_LPR_SERVERS          9
#define DHO_IMPRESS_SERVERS     10
#define DHO_RESOURCE_LOCATION_SERVERS   11
#define DHO_HOST_NAME           12
#define DHO_BOOT_SIZE           13
#define DHO_MERIT_DUMP          14
#define DHO_DOMAIN_NAME         15
#define DHO_SWAP_SERVER         16
#define DHO_ROOT_PATH           17
#define DHO_EXTENSIONS_PATH     18
#define DHO_IP_FORWARDING       19
#define DHO_NON_LOCAL_SOURCE_ROUTING 20
#define DHO_POLICY_FILTER            21
#define DHO_MAX_DGRAM_REASSEMBLY     22
#define DHO_DEFAULT_IP_TTL           23
#define DHO_PATH_MTU_AGING_TIMEOUT   24
#define DHO_PATH_MTU_PLATEAU_TABLE   25
#define DHO_INTERFACE_MTU            26
#define DHO_ALL_SUBNETS_LOCAL        27
#define DHO_BROADCAST_ADDRESS        28
#define DHO_PERFORM_MASK_DISCOVERY   29
#define DHO_MASK_SUPPLIER            30
#define DHO_ROUTER_DISCOVERY         31
#define DHO_ROUTER_SOLICITATION_ADDRESS 32
#define DHO_STATIC_ROUTES           33
#define DHO_TRAILER_ENCAPSULATION   34
#define DHO_ARP_CACHE_TIMEOUT       35
#define DHO_IEEE802_3_ENCAPSULATION 36
#define DHO_DEFAULT_TCP_TTL         37
#define DHO_TCP_KEEPALIVE_INTERVAL  38
#define DHO_TCP_KEEPALIVE_GARBAGE   39
#define DHO_NIS_DOMAIN              40
#define DHO_NIS_SERVERS             41
#define DHO_NTP_SERVERS             42
#define DHO_VENDOR_ENCAPSULATED_OPTIONS 43
#define DHO_NETBIOS_NAME_SERVERS    44
#define DHO_NETBIOS_DD_SERVER       45
#define DHO_NETBIOS_NODE_TYPE       46
#define DHO_NETBIOS_SCOPE           47
#define DHO_FONT_SERVERS            48
#define DHO_X_DISPLAY_MANAGER       49
#define DHO_DHCP_REQUESTED_ADDRESS  50
#define DHO_DHCP_LEASE_TIME         51
#define DHO_DHCP_OPTION_OVERLOAD    52
#define DHO_DHCP_MESSAGE_TYPE       53
#define DHO_DHCP_SERVER_IDENTIFIER  54
#define DHO_DHCP_PARAMETER_REQUEST_LIST 55
#define DHO_DHCP_MESSAGE            56
#define DHO_DHCP_MAX_MESSAGE_SIZE   57
#define DHO_DHCP_RENEWAL_TIME       58
#define DHO_DHCP_REBINDING_TIME     59
#define DHO_VENDOR_CLASS_IDENTIFIER 60
#define DHO_DHCP_CLIENT_IDENTIFIER  61
#define DHO_NWIP_DOMAIN_NAME        62
#define DHO_NWIP_SUBOPTIONS         63
#define DHO_USER_CLASS              77
#define DHO_FQDN                    81
#define DHO_DHCP_AGENT_OPTIONS      82
#define DHO_SUBNET_SELECTION       118  // RFC3011
/* The DHO_AUTHENTICATE option is not a standard yet, so I've
   allocated an option out of the "local" option space for it on a
   temporary basis.  Once an option code number is assigned, I will
   immediately and shamelessly break this, so don't count on it
   continuing to work. */
#define DHO_AUTHENTICATE           210
#define DHO_END                    255

// DHCP message types
#define DHCPDISCOVER 1
#define DHCPOFFER    2
#define DHCPREQUEST  3
#define DHCPDECLINE  4
#define DHCPACK      5
#define DHCPNAK      6
#define DHCPRELEASE  7
#define DHCPINFORM   8

// Relay Agent Information option subtypes
#define RAI_CIRCUIT_ID  1
#define RAI_REMOTE_ID   2
#define RAI_AGENT_ID    3

// FQDN suboptions
#define FQDN_NO_CLIENT_UPDATE 1
#define FQDN_SERVER_UPDATE    2
#define FQDN_ENCODED          3
#define FQDN_RCODE1           4
#define FQDN_RCODE2           5
#define FQDN_HOSTNAME         6
#define FQDN_DOMAINNAME       7
#define FQDN_FQDN             8
#define FQDN_SUBOPTION_COUNT  8

#define ETH_ALEN           6  // octets in one ethernet header
#define DHCP_DEST_PORT    67
#define DHCP_SOURCE_PORT  68

namespace net
{
  static const IP4::addr INADDR_ANY   {{0}};
  static const IP4::addr INADDR_BCAST {{0xFF, 0xFF, 0xFF, 0xFF}};
  
  inline dhcp_option_t* get_option(uint8_t* option)
  {
    return (dhcp_option_t*) option;
  }
  
	void DHClient::negotiate()
  {
    // create DHCP discover packet
    debug("* Negotiating IP address through DHCP discover\n");
    // create a random session ID
    uint32_t xxxxxx;
    this->xid = xxxxxx;
    debug("* DHCP session ID %u (size=%u)\n", xid, sizeof(xid));
    
    const size_t packetlen = sizeof(dhcp_packet_t);
    char packet[packetlen];
    
    dhcp_packet_t* dhcp = (dhcp_packet_t*) packet;
    dhcp->op    = BOOTREQUEST;
    dhcp->htype = HTYPE_ETHER;
    dhcp->hlen  = ETH_ALEN;
    dhcp->hops  = 0;
    dhcp->xid   = htonl(this->xid);
    dhcp->secs  = 0;
    dhcp->flags = htons(BOOTP_UNICAST);
    dhcp->ciaddr = INADDR_ANY;
    dhcp->yiaddr = INADDR_ANY;
    dhcp->siaddr = INADDR_ANY;
    dhcp->giaddr = INADDR_ANY;
    
    // copy our hardware address to chaddr field
    memset(dhcp->chaddr, 0, dhcp_packet_t::CHADDR_LEN);
    memcpy(dhcp->chaddr, &stack.link_addr(), ETH_ALEN);
    // zero server, file and options
    memset(dhcp->sname, 0, dhcp_packet_t::SNAME_LEN + dhcp_packet_t::FILE_LEN);
    
    dhcp->magic[0] =  99;
    dhcp->magic[1] = 130;
    dhcp->magic[2] =  83;
    dhcp->magic[3] =  99;
    
    dhcp_option_t* opt = get_option(dhcp->options + 0);
    // DHCP discover
    opt->code   = DHO_DHCP_MESSAGE_TYPE;
    opt->length = 1;
    opt->val[0] = DHCPDISCOVER;
    // DHCP client identifier
    opt = get_option(dhcp->options + 3);
    opt->code   = DHO_DHCP_CLIENT_IDENTIFIER;
    opt->length = 7;
    opt->val[0] = HTYPE_ETHER;
    memcpy(&opt->val[1], &stack.link_addr(), ETH_ALEN);
    // Parameter Request List
    opt = get_option(dhcp->options + 12);
    opt->code   = DHO_DHCP_PARAMETER_REQUEST_LIST;
    opt->length = 4;
    opt->val[0] = 0x01;
    opt->val[1] = 0x0f;
    opt->val[2] = 0x03;
    opt->val[3] = 0x06;
    // END
    opt = get_option(dhcp->options + 18);
    opt->code   = DHO_END;
    opt->length = 0;
    
    ////////////////////////////////////////////////////////
    auto& socket = stack.udp().bind(DHCP_SOURCE_PORT);
    /// broadcast our DHCP plea as 0.0.0.0:67
    socket.bcast(INADDR_ANY, DHCP_DEST_PORT, packet, packetlen);
    
    socket.onRead(
    [this] (SocketUDP& sock, IP4::addr addr, UDP::port port, 
            const char* data, int len) -> int
    {
      printf("Received data on %d from %s:%d (should be %d)\n",
          DHCP_SOURCE_PORT, addr.str().c_str(), port, DHCP_DEST_PORT);
      
      if (addr == INADDR_BCAST && port == DHCP_DEST_PORT)
          this->offer(sock, data, len);
      
      return -1;
    });
  }
  
  void DHClient::offer(SocketUDP& sock, const char* data, int datalen)
  {
    printf("Reading offered DHCP information\n");
    const dhcp_packet_t* dhcp = (const dhcp_packet_t*) data;
    
    printf("Offer xid: %u  Our xid: %u", dhcp->xid, this->xid);
    printf("DHCP IP: %s  NETMASK: %s", 
        dhcp->ciaddr.str().c_str(), dhcp->yiaddr.str().c_str());
    
    // form a response
    const size_t packetlen = sizeof(dhcp_packet_t);
    char packet[packetlen];
    
    dhcp_packet_t* resp = (dhcp_packet_t*) packet;
    // copy most of the offer into our response
    memcpy(resp, dhcp, sizeof(dhcp_packet_t));
    
    // send response
    sock.bcast(INADDR_ANY, DHCP_DEST_PORT, packet, packetlen);
  }
}
