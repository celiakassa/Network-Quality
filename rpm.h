
struct instan_aggregate_node{
  int flag;
  long goodput;
  struct instan_aggregate_node *next;
};

typedef struct instan_aggregate_node * Instan_aggregate;
Instan_aggregate init_instan_list(void);
Instan_aggregate addAggregate(long goodput, Instan_aggregate headI);
Instan_aggregate delAggregate(Instan_aggregate headI);
long get_prev_goodput(Instan_aggregate ia);
