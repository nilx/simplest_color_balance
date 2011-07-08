/* balance_lib.c */
unsigned char *balance_u8(unsigned char *data, size_t size, size_t nb_min, size_t nb_max);
float *balance_f32(float *data, size_t size, size_t nb_min, size_t nb_max);
float *idiv_f32(float *a, const float *b, size_t size);
float *imul_f32(float *a, const float *b, size_t size);
float *imuls_f32(float *a, const float b, size_t size);
float *iadd_f32(float *a, const float *b, size_t size);
