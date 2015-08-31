typedef struct Connection {
  std::string ip;
  bool alive;
	int udp_credits;
  int tcp_credits;
  int pos_udp;
  int pos_tcp;
	bool _opoznienia;
	bool _ssh;
	int udp[10];
	int ssh[10];
	int icmp[10];
} Connection;
