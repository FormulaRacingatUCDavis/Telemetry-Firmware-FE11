class Packet {
 public:
  static const size_t length = 16;
  char* validation;
  int data_id;
  short* data;
  unsigned long time;
  Packet(char* vd, int id, short* d, unsigned long t) : validation(vd), data_id(id), data(d), time(t) {};
};