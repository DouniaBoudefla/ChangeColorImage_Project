#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "pngioh_mod.h"

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
	
	
    struct palette *pal;
    struct buffer *pal_conv;
    pal=new_palette(0);
    pal_conv=new_buffer(0);
    /* palette de couleur de l'image (format indexé)*/
    for(int i = 0; i < image->height; i++) {
      for(int j = 0; j < image->width; j++) {
	dedans( image->data[i][j * 4],  image->data[i][j * 4+ 1], image->data[i][j * 4 + 2], pal); 
      }
    }
	
	
    if (pal->r->len >= 256 ) {
      printf("Error, %d de couleur\n", pal->r->len );
      goto End;
    }
	
    /* Conversion de la palette de l'image dans la palette que l'on souhaite */
    pal_conv=conversion(pal,pal);
	
    for(int i = 0; i < image->height; i++) {
      for(int j = 0; j < image->width; j++) {
	for (int k = 0; k < pal_conv->len; k++) {
	  if ( image->data[i][j * 4 ] ==  pal->r->a[k] && image->data[i][j * 4+ 1] == pal->g->a[k] && image->data[i][j * 4 + 2] == pal->b->a[k] ) {
	    image2->data[i][j * 4] = pal->r->a[pal_conv->a[k]];
	    image2->data[i][j * 4 + 1] = pal->g->a[pal_conv->a[k]];
	    image2->data[i][j * 4 + 2] = pal->g->a[pal_conv->a[k]];
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
	
 End:
    destroy_buffer(pal_conv);
    destroy_palette(pal);

    
    free_image(image);
    free_image(image2);
 

    return 0;
}
