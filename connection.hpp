typedef struct Connection {
  std::string ip;
  bool alive;
	int active;
  int pos;
	bool _opoznienia;
	bool _ssh;
	int udp[10];
	int ssh[10];
	int icmp[10];
} Connection;
