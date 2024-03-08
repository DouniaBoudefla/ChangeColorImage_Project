#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pngioh_mod.h"

void
free_image(struct image *image)
{
    for(int i = 0; i < image->height; i++)
        free(image->data[i]);
    free(image->data);
    free(image);
}

void
free_pal_image(struct pal_image *image)
{
    for(int i = 0; i < image->height; i++)
        free(image->data[i]);
    free(image->data);
    free(image->pal);
    free(image);
}


struct image *
read_png(char *filename) {
    FILE *f;
    unsigned char header[8];
    png_structp pngp = NULL;
    png_infop infop = NULL;
    int numrows = 0;
    int ctype, depth;
    unsigned char **rows = NULL;
    struct image *image = NULL;
    int rc;

    f = fopen(filename, "r");
    if(f == NULL) {
        perror("fopen");
        return NULL;
    }

    rc = fread(header, 1, 8, f);
    if(rc != 8) {
        perror("read(header)");
        goto fail;
    }

    if(png_sig_cmp(header, 0, 8) != 0) {
        fprintf(stderr, "Not a PNG file.\n");
        goto fail;
    }

    pngp = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     NULL, NULL, NULL);
    if(pngp == NULL)
        goto fail;

    infop = png_create_info_struct(pngp);
    if(infop == NULL)
        goto fail;

    if(setjmp(png_jmpbuf(pngp))) {
        fprintf(stderr, "Error while reading PNG file.\n");
        goto fail;
    }

    png_init_io(pngp, f);
    png_set_sig_bytes(pngp, 8);

    png_read_info(pngp, infop);

    ctype = png_get_color_type(pngp, infop);
    if(ctype != (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA)) {
        fprintf(stderr, "Bad color type %d.\n", ctype);
        goto fail;
    }
    depth = png_get_bit_depth(pngp, infop);
    if(depth != 8) {
        fprintf(stderr, "Color depth is %d, expected 8.\n", depth);
        goto fail;
    }

    numrows = png_get_image_height(pngp, infop);
    rows = calloc(numrows, sizeof(unsigned char *));
    if(rows == NULL)
        goto fail;
    for(int i = 0; i < numrows; i++) {
        rows[i] = malloc(png_get_rowbytes(pngp, infop));
        if(rows[i] == NULL)
            goto fail;
    }

    png_read_image(pngp, rows);

    image = malloc(sizeof(struct image));
    if(image == NULL)
        goto fail;
    image->data = rows;
    image->height = png_get_image_height(pngp, infop);
    image->width = png_get_image_width(pngp, infop);

    png_destroy_read_struct(&pngp, &infop, NULL);
    fclose(f);

    return image;

 fail:
    png_destroy_read_struct(&pngp, &infop, NULL);

    if(image != NULL) {
        free(image->data);
        free(image);
    }

    if(rows != NULL) {
        for(int i = 0; i < numrows; i++)
            free(rows[i]);
        free(rows);
    }

    fclose(f);
    return NULL;
}

int
write_png(char *file, const struct image *image)
{
    FILE *f;
    png_structp pngp = NULL;
    png_infop infop = NULL;

    f = fopen(file, "w");
    if(f == NULL) {
        perror("fopen");
        return -1;
    }

    pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                   NULL, NULL, NULL);
    if(pngp == NULL)
        goto fail;

    infop = png_create_info_struct(pngp);
    if(infop == NULL)
        goto fail;

    if(setjmp(png_jmpbuf(pngp))) {
        fprintf(stderr, "Unable to write PNG file.\n");
        goto fail;
    }

    png_init_io(pngp, f);

    png_set_IHDR(pngp, infop,
                 image->width, image->height,
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_set_rows(pngp, infop, image->data);

    png_write_png(pngp, infop, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_write_struct(&pngp, &infop);
    fclose(f);
    return 1;

 fail:
    png_destroy_write_struct(&pngp, &infop);
    fclose(f);
    return -1;
}

int
write_pal_png(char *file, const struct pal_image *image)
{
    FILE *f;
    png_structp pngp = NULL;
    png_infop infop = NULL;
    png_colorp pal = NULL;

    f = fopen(file, "w");
    if(f == NULL) {
        perror("fopen");
        return -1;
    }

    pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                   NULL, NULL, NULL);
    if(pngp == NULL)
        goto fail;

    infop = png_create_info_struct(pngp);
    if(infop == NULL)
        goto fail;

    if(setjmp(png_jmpbuf(pngp))) {
        fprintf(stderr, "Unable to write PNG file.\n");
        goto fail;
    }

    png_init_io(pngp, f);

    png_set_IHDR(pngp, infop,
                 image->width, image->height,
                 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_set_rows(pngp, infop, image->data);

    pal = malloc(image->pal_len * sizeof(png_color));
    if(pal == NULL)
        goto fail;

    for(int i = 0; i < image->pal_len; i++) {
        pal[i].red = image->pal[3 * i];
        pal[i].green = image->pal[3 * i + 1];
        pal[i].blue = image->pal[3 * i + 2];
    }
    png_set_PLTE(pngp, infop, pal, image->pal_len);

    png_write_png(pngp, infop, PNG_TRANSFORM_IDENTITY, NULL);

    free(pal);
    pal = NULL;
    png_destroy_write_struct(&pngp, &infop);
    fclose(f);
    return 1;

 fail:
    free(pal);
    png_destroy_write_struct(&pngp, &infop);
    fclose(f);
    return -1;
}

/* Palette */

struct buffer *new_buffer(int cap) {
  struct buffer *p;
  p = malloc(sizeof(struct buffer));
  int *q;
  q=malloc(cap*sizeof(int));
  (*p).a = q;
  (*p).len = 0;
  (*p).cap = cap;
  return p;
};

int extend(struct buffer *s){
  int n;
  if (s->cap > 0){
    n=2*s->cap;
  } else {
    n=4;
  }
  int *q;
  q=malloc(n*sizeof(int));
  if (q == NULL){
    return -1;
  }
  int i;
  for(i=0; i < s->len ; i++) {
   q[i]=s->a[i];
  }
  free(s->a);
  s->a = q;
  s->cap = n;
  return 1;
}

struct palette *new_palette(int n) {
	struct palette *pal;
	pal=malloc(sizeof(struct palette));
	pal->r=new_buffer(n);
	pal->g=new_buffer(n);
	pal->b=new_buffer(n);
	return pal;
}

void add_buff(struct buffer *p, int n) {
	if (p->len>=p->cap) {
		while (p->len>=p->cap){
			extend(p);
		}
		p->a[p->len]=n;
		p->len+=1;
	} else {
		p->a[p->len]=n;
		p->len+=1;
	}
}

int Entier(double d) {
	int i=0;
	while (i<d) {
		i++;
	}
	return i-1;
}

struct palette *saturee() {
	struct palette *pal;
	pal=new_palette(8);
	for (int i=0; i<2;i++) {
		for (int j=0; j<2; j++) {
			for (int k=0; k<2; k++) {
				add_buff(pal->r,255*i);
				add_buff(pal->g,255*j);
				add_buff(pal->b,255*k);
			}
		}
	}
	return pal;
}

struct palette *quatre_quatre_quatre() {
	struct palette *pal;
	pal=new_palette(64);
	for (int i=0; i<4;i++) {
		for (int j=0; j<4; j++) {
			for (int k=0; k<4; k++) {
				add_buff(pal->r,85*i);
				add_buff(pal->g,85*j);
				add_buff(pal->b,85*k);
			}
		}
	}
	return pal;
}

struct palette *six_six_six() {
	struct palette *pal;
	pal=new_palette(216);
	for (int i=0; i<6;i++) {
		for (int j=0; j<6; j++) {
			for (int k=0; k<6; k++) {
				add_buff(pal->r,51*i);
				add_buff(pal->g,51*j);
				add_buff(pal->b,51*k);
			}
		}
	}
	return pal;
}

struct palette *six_sept_six() {
	struct palette *pal;
	pal=six_six_six();
	for (int i=0; i<6;i++) {
		for (int k=0; k<6; k++) {
			add_buff(pal->r,51*i);
			add_buff(pal->g,212);
			add_buff(pal->b,51*k);
		}
	}
	return pal;
}

struct palette *noir_blanc() {
	struct palette *pal;
	pal=new_palette(2);
	for (int i=0; i<2;i++) {
		add_buff(pal->r,255*i);
		add_buff(pal->g,255*i);
		add_buff(pal->b,255*i);
	}
	return pal;
}

struct palette *gris() {
	struct palette *pal;
	pal=new_palette(256);
	for (int i=0; i<256;i++) {
		add_buff(pal->r,i);
		add_buff(pal->g,i);
		add_buff(pal->b,i);
	}
	return pal;
}

struct palette *CGA() {
	struct palette *pal;
	pal=new_palette(16);
	for (int i=0; i<2;i++) {
		for (int j=0; j<2; j++) {
			for (int k=0; k<2; k++) {
				for (int l=0; l<2; l++) {
					if (i!=0 && j!=0 && k!=0 && l!=1) {
						add_buff(pal->r,Entier(127.5*i+127.5*l));
						add_buff(pal->g,Entier(127.5*j+127.5*l));
						add_buff(pal->b,Entier(127.5*k+127.5*l));
					}
				}
			}
		}
	}
	add_buff(pal->r,192);
	add_buff(pal->g,192);
	add_buff(pal->b,192);
	return pal;
}

int minimum(struct buffer *p) {
  int min=p->a[0];
  int b=0;
  for (int i=0; i<p->len; i++) {
    if (p->a[i]<min) {
      min=p->a[i];
      b=i;
    }
  }
  return b;
}

double distance(int r, int g, int b, int R, int G, int B) {
  int d_carre;
  d_carre=(r-R)*(r-R) + (g-G)*(g-G) + (b-B)*(b-B);
  double d;
  d=sqrt(d_carre);
  return d;
}

struct buffer *conversion(struct palette *pal, struct palette *conv) {
	struct buffer *pal_conv=new_buffer(pal->r->len);
	for (int i=0; i<pal->r->len; i++) {
	  double d;
	  struct buffer *tab=new_buffer(conv->r->len);
	  for (int j=0; j<conv->r->len; j++) {	
	    d=distance(pal->r->a[i],pal->g->a[i],pal->b->a[i],conv->r->a[j],conv->g->a[j],conv->b->a[j]);
	    tab->a[tab->len]=d;
	    tab->len++;
	  }
	  add_buff(pal_conv,minimum(tab));
	  free(tab->a);
	  free(tab);
	}
	return pal_conv;
};

int dedans(int r, int g, int b, struct palette *pal) {   
	int t=0;
    for (int i=0; i< pal->r->len; i++) { /*voir si la couleur (r,g,b) est dedans*/
        if (r==pal->r->a[i] && g==pal->g->a[i] && b==pal->b->a[i]) {
            t=1;
        } 
    }
	if (t==0) {
		add_buff(pal->r,r);
		add_buff(pal->g,g);
		add_buff(pal->b,b);
	}
    return 1;
}


void destroy_buffer(struct buffer *s){
  free(s->a);
  free(s);
}

void destroy_palette(struct palette *pal) {
    destroy_buffer(pal->r);
    destroy_buffer(pal->g);
    destroy_buffer(pal->b);
    free(pal);
}

