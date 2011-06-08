/* normalize_histo_lib.c */

 void minmax_histo_u8(unsigned char *data, size_t size, size_t nb_min, size_t nb_max,  unsigned char *ptr_min, unsigned char *ptr_max);

unsigned char *normalize_histo_u8(unsigned char *data, size_t size, unsigned char target_min, unsigned char target_max, size_t flat_nb_min, size_t flat_nb_max);

unsigned char *normalize_histo_u8_gray(unsigned char *data, size_t size, unsigned char target_min, unsigned char target_max,  unsigned char min, unsigned char max );

void color_u8(unsigned char *data_out, unsigned char *data, unsigned char *gray, unsigned char *gray1, size_t dim);

