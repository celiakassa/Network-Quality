enum test_type{
   DNS,
   TCP
};

struct mesure{
  long  duration;
  enum test_type type;
};

typedef struct mesure mesure;
long dns(char url_str[]);
long tcp(char url_str[]);
