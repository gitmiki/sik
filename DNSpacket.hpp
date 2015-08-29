typedef struct DNSHeader {
  uint16_t ID;
  uint8_t rd :1; // recursion desired
  uint8_t tc :1; // truncated message
  uint8_t aa :1; // authorative answer
  uint8_t opcode :4; // purpose of message
  uint8_t qr :1; // query/reponse flag
  uint8_t rcode :4; // response code
  uint8_t cd :1; // checking disabled
  uint8_t ad :1; // authenticated DATA
  uint8_t z :1; // its z! reserved
  uint8_t ra :1; // recursion avalaible
  //uint16_t Flags;
  uint16_t qdcount; //No. of Questions
  uint16_t ancount; //No. of Answers
  uint16_t nscount; //No. of Authority
  uint16_t arcount; //No. of resource entries
} DNSHeader;

typedef struct DNSQuery {
  uint16_t type;
  uint16_t qclass; //should be 1 since we are on internet
} DNSQuery;

typedef struct RRecord {
  //unsigned char *name; /// /??????????? jak ityp ???????
  uint16_t rtype;
  uint16_t rclass;
  uint16_t ttl;
  uint16_t rdlength;
  //unsigned char *rdata; //// ???????????????? jaki typ??? ?///////
} RRecord;

typedef struct DNSpacket {
  DNSHeader header;
  DNSQuery query;
  RRecord *answer;
  RRecord *authority;
  RRecord *additional;
} DNSpacket;
