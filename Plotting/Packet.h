class Packet {
 public:
  static const size_t length = 14;
  int data_id;
  short* data;
  unsigned long time;
  Packet(int id, short* d, unsigned long t) : data_id(id), data(d), time(t) {};
};