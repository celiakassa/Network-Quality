enum test_type{
   DNS,
   TCP,
   TLS, 
   DWN
};

struct mesure{
  long  duration;
  enum test_type type;
};

typedef struct mesure mesure;
long dns(char url_str[]);
long tcp(char url_str[]);
long tls(char url_str[]);

