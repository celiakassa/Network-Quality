struct mesure{
  double  duration;
  char test_type;
 
};

typedef struct mesure mesure;
int tcp(char url_str[]);
int dns(char url_str[]);
