/* colorspace_lib.c */
void rgb2hsl(const float *rgb, float *hsl, size_t size);
void hsl2rgb(const float *hsl, float *rgb, size_t size);
void rgb2hsv(const float *rgb, float *hsv, size_t size);
void hsv2rgb(const float *hsv, float *rgb, size_t size);
void rgb2hsi(const float *rgb, float *hsi, size_t size);
void hsi2rgb(const float *hsi, float *rgb, size_t size);
void rgb2ycbcr(const float *rgb, float *ycbcr, size_t size);
void ycbcr2rgb(const float *ycbcr, float *rgb, size_t size);
