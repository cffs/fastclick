#ifndef CLICK_ETX_HH
#define CLICK_ETX_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <click/vector.hh>
#include <click/hashmap.hh>
#include "linktable.hh"
#include "arptable.hh"
#include "srcr.hh"
CLICK_DECLS

/*
 * =c
 * EETX(IP, ETH, ETHERTYPE, [LS link-state-element])
 * =d
 * DSR-inspired end-to-end ad-hoc routing protocol.
 * Input 0: ethernet packets 
 * Input 1: IP packets from higher layer, w/ ip addr anno.
 * Output 0: ethernet packets to device (protocol)
 * Output 1: ethernet packets to device (data)
 *
 */


class ETX : public Element {
 public:
  
  ETX();
  ~ETX();
  
  const char *class_name() const		{ return "ETX"; }
  const char *processing() const		{ return PUSH; }
  int initialize(ErrorHandler *);
  ETX *clone() const;
  int configure(Vector<String> &conf, ErrorHandler *errh);


  /* handler stuff */
  void add_handlers();
  static int static_clear(const String &arg, Element *e,
			  void *, ErrorHandler *errh); 
  void clear();

  static int static_start(const String &arg, Element *e,
			  void *, ErrorHandler *errh); 
  void start(IP6Address dst);

  static String static_print_stats(Element *e, void *);
  String print_stats();


  void push(int, Packet *);
  void run_timer();



  static unsigned int jiff_to_ms(unsigned int j)
  { return (j * 1000) / CLICK_HZ; }

  static unsigned int ms_to_jiff(unsigned int m)
  { return (CLICK_HZ * m) / 1000; }


  static String ETX::route_to_string(Vector<IP6Address> s);
private:

  class Query {
  public:
    Query() {memset(this, 0, sizeof(*this)); }
    Query(IP6Address ip) {memset(this, 0, sizeof(*this)); _ip = ip;}
    IP6Address _ip;
    u_long _seq;
    u_short _metric;
    struct timeval _last_query;

  };

  
  // List of query sequence #s that we've already seen.
  class Seen {
  public:
    IP6Address _src;
    IP6Address _dst;
    u_long _seq;
    int _count;
    struct timeval _when; /* when we saw the first query */
    Seen(IP6Address src, IP6Address dst, u_long seq ) {
      _src = src; _dst = dst; _seq = seq; _count = 0;
    }
    Seen();
  };

  typedef HashMap<IP6Address, Query> QueryTable;
  QueryTable _queries;

  Vector<Seen> _seen;

  int MaxSeen;   // Max size of table of already-seen queries.
  int MaxHops;   // Max hop count for queries.
  struct timeval _reply_wait;
  struct timeval _query_wait;

  u_long _seq;      // Next query sequence number to use.
  Timer _timer;
  IP6Address _ip;    // My IP address.
  EtherAddress _en; // My ethernet address.
  uint16_t _et;     // This protocol's ethertype

  IP6Address _gw;
  IP6Address _bcast_ip;
  bool _is_gw;

  class SRCR *_srcr;
  class LinkTable *_link_table;
  class LinkStat *_link_stat;
  class ARPTable *_arp_table;

  // Statistics for handlers.
  int _num_queries;
  int _bytes_queries;
  int _num_replies;
  int _bytes_replies;






  static void static_reply_hook(Timer *t, void *v) 
   { ((ETX *) v)->reply_hook(t); }


  int find_dst(IP6Address ip, bool create);
  EtherAddress find_arp(IP6Address ip);
  void got_arp(IP6Address ip, EtherAddress en);
  u_short get_metric(IP6Address other);
  void got_sr_pkt(Packet *p_in);
  void start_query(IP6Address);
  void process_query(struct sr_pkt *pk);
  void forward_query(Seen s, Vector<IP6Address> hops, Vector<u_short> metrics);
  void start_reply(IP6Address src, IP6Address dst, u_long seq);
  void forward_reply(struct sr_pkt *pk);
  void got_reply(struct sr_pkt *pk);
  void start_data(const u_char *data, u_long len, Vector<IP6Address> r);
  void send(WritablePacket *);

  void reply_hook(Timer *t);
  void update_link(IP6Pair p, u_short m, unsigned int now);
  void etx_assert_(const char *, int, const char *) const;

};


CLICK_ENDDECLS
#endif
