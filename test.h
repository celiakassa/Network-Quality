enum test_type{
   DNS,
   TCP,
   TLS, 
   DWN,
   UP
};

struct mesure{
  long  duration;
  enum test_type type;
};

typedef struct mesure mesure;
long dns(char url_str[]);
long tcp(char url_str[]);
long tls(char url_str[]);
long down(char param1[], char param2[]);
long up(char param1[], char param2[]);
