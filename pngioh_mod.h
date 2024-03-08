struct image {
    unsigned char **data;
    int height, width;
};

struct pal_image {
    unsigned char **data;
    int height, width;
    unsigned char *pal;
    int pal_len;
};

void free_image(struct image *image);
void free_pal_image(struct pal_image *image);
struct image *read_png(char *filename);
int write_png(char *file, const struct image *image);
int write_pal_png(char *file, const struct pal_image *image);



struct palette {
	struct buffer *r;
	struct buffer *g;
	struct buffer *b;
};

struct buffer {
  int *a;
  int len;
  int cap;
};

struct buffer *new_buffer(int cap);

int extend(struct buffer *s);

struct palette *new_palette(int n);

void add_buff(struct buffer *p, int n);

int Entier(double d);

struct palette *saturee();

struct palette *quatre_quatre_quatre();

struct palette *six_six_six();

struct palette *six_sept_six();

struct palette *noir_blanc();

struct palette *gris();

struct palette *CGA();

struct buffer *conversion(struct palette *pal, struct palette *conv);

int dedans(int r, int g, int b, struct palette *pal);

double distance(int r, int g, int b, int R, int G, int B);

void destroy_buffer(struct buffer *p);

void destroy_palette(struct palette *pal);
