#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "pngioh_mod.h"

int maximum(struct buffer *p) {
  int max=p->a[0];
  int b=0;
  for (int i=0; i<p->len; i++) {
    if (p->a[i]>max) {
      max=p->a[i];
      b=i;
    }
  }
  return b;
}


struct buffer *max_color(struct buffer *p, int n) {
  int i=0;
  struct buffer *q = new_buffer(n);
  while (i<n) {
	p->a[maximum(p)]=0;
	add_buff(q,maximum(p));
    i++;
  }
  return q;
};




struct palette *Palette_indiceMax(struct buffer *p, struct palette *pal) {
  struct palette *ret=new_palette(0);
  for (int i=0; i<p->len; i++) {
    if (p->a[i]==0) {
      add_buff(ret->r, pal->r->a[i]);
      add_buff(ret->g, pal->g->a[i]);
      add_buff(ret->b, pal->b->a[i]);
    }
  }
  return ret;
};

		
int
main(int argc, char **argv)
{
  struct image *image, *image2;
  int rc;

  while(1) {
    int opt;

    opt = getopt(argc, argv, "");
    if(opt < 0)
      break;

    switch(opt) {
    default:
      fprintf(stderr, "Unknown option.\n");
      return 1;
    }
  }

  if(argc != optind + 2) {
    fprintf(stderr, "Requires 2 parameters.\n");
    return 1;
  }

  image = read_png(argv[optind]);
  if(image == NULL) {
    fprintf(stderr, "Couldn't read %s\n", argv[optind]);
    return 1;
  }

  image2 = calloc(1, sizeof(struct image));
  if(image2 == NULL) {
    fprintf(stderr, "Couldn't allocate image2.\n");
    return 1;
  }

  image2->width = image->width;
  image2->height = image->height;
  image2->data = malloc(image->height * sizeof(unsigned char*));
  if(image2->data == NULL) {
    fprintf(stderr, "Couldn't allocate data 2.\n");
    return 1;
  }
  for(int i = 0; i < image->height; i++) {
    image2->data[i] = malloc(4 * image->width);
    if(image2->data[i] == NULL) {
      fprintf(stderr, "Couldn't allocate data 2.\n");
      return 1;
    }
  }
  /* Début modification */
	
	
  struct buffer *c;
  struct palette *pal;
  struct palette *pal_uti;
  struct buffer *pal_conv;
  pal=new_palette(0);
  /* palette de couleur de l'image (format indexé)*/
  for(int i = 0; i < image->height; i++) {
    for(int j = 0; j < image->width; j++) {
      dedans( image->data[i][j * 4],  image->data[i][j * 4+ 1], image->data[i][j * 4 + 2], pal);
      }
}
c=new_buffer(pal->r->len);
for (int i=0; i<pal->r->len; i++) {
  add_buff(c,0);
 }
for(int i = 0; i < image->height; i++) {
  for(int j = 0; j < image->width; j++) {
    for (int k=0; k<pal->r->len; k++) {
      if ( image->data[i][j * 4 ] ==  pal->r->a[k] && image->data[i][j * 4+ 1] == (unsigned char) pal->g->a[k] && image->data[i][j * 4 + 2] == (unsigned char) pal->b->a[k] ) {
	c->a[k]++;
	break;
      }
    }
  }
 }



int n;
printf("combien de couleur max dans l'image ?\n");
scanf("%d", &n);
c=max_color(c, n);
pal_uti=Palette_indiceMax(c, pal);


/* Conversion de la palette de l'image dans la palette que l'on souhaite */
pal_conv = conversion(pal,pal_uti);
	
	
for(int i = 0; i < image->height; i++) {
  for(int j = 0; j < image->width; j++) {
    for (int k = 0; k < pal_conv->len; k++) {
      if ( image->data[i][j * 4 ] ==  pal->r->a[k] && image->data[i][j * 4+ 1] == (unsigned char) pal->g->a[k] && image->data[i][j * 4 + 2] == (unsigned char) pal->b->a[k] ) {
	image2->data[i][j * 4] = pal_uti->r->a[pal_conv->a[k]];
	image2->data[i][j * 4 + 1] = pal_uti->g->a[pal_conv->a[k]];
	image2->data[i][j * 4 + 2] = pal_uti->g->a[pal_conv->a[k]];
	image2->data[i][j * 4 + 3] = image->data[i][j * 4 + 3];
      }
    }
  }
 }
	
/* Fin modification */
rc = write_png(argv[optind + 1], image2);
if(rc < 0) {
  fprintf(stderr, "Couldn't write %s.\n", argv[optind + 1]);
  return 1;
 }

free_image(image);
free_image(image2);
destroy_palette(pal_uti);
destroy_palette(pal);
destroy_buffer(pal_conv);
destroy_buffer(c);

return 0;
}
