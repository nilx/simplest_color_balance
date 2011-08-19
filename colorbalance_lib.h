/* colorbalance_lib.c */
float *colorbalance_rgb_f32(float *rgb, size_t size, size_t nb_min, size_t nb_max);
float *colorbalance_hsl_f32(float *rgb, size_t size, size_t nb_min, size_t nb_max);
float *colorbalance_hsv_f32(float *rgb, size_t size, size_t nb_min, size_t nb_max);
float *colorbalance_irgb_bounded_f32(float *rgb, size_t size, size_t nb_min, size_t nb_max);
float *colorbalance_irgb_adjusted_f32(float *rgb, size_t size, size_t nb_min, size_t nb_max);
