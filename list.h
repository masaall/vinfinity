
struct list_elem {
	struct list_elem *prev;
	struct list_elem *next;
};

struct list {
	struct list_elem base;
};
