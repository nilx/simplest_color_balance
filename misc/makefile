# Copyright 2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
#
# Copying and distribution of this file, with or without
# modification, are permitted in any medium without royalty provided
# the copyright notice and this notice are preserved.  This file is
# offered as-is, without any warranty.

M4V	= RGB_64colors.rgb.00.xx.m4v RGB_64colors.rgb.xx.00.m4v RGB_64colors.rgb.xx.xx.m4v \
		RGB_64colors.hsi.00.xx.m4v RGB_64colors.hsi.xx.00.m4v RGB_64colors.hsi.xx.xx.m4v \
		RGB_64colors.hsl.00.xx.m4v RGB_64colors.hsl.xx.00.m4v RGB_64colors.hsl.xx.xx.m4v \
		RGB_64colors.hsv.00.xx.m4v RGB_64colors.hsv.xx.00.m4v RGB_64colors.hsv.xx.xx.m4v \
		RGB_64colors.irgb_bounded.00.xx.m4v RGB_64colors.irgb_bounded.xx.00.m4v RGB_64colors.irgb_bounded.xx.xx.m4v \
		RGB_64colors.irgb_adjusted.00.xx.m4v RGB_64colors.irgb_adjusted.xx.00.m4v RGB_64colors.irgb_adjusted.xx.xx.m4v \
		RGB_64colors.diff.00.xx.m4v RGB_64colors.diff.xx.00.m4v RGB_64colors.diff.xx.xx.m4v \
		RGB_64colors.all.00.xx.m4v RGB_64colors.all.xx.00.m4v RGB_64colors.all.xx.xx.m4v

IMAX	= 99
IMAX2	= 49

all	: ${M4V}

RGB_%colors.png	: make_rgb_%colors.c
	./$< > $@

../balance	:
	$(MAKE) -B -C ../ balance

RGB_64colors/diff	: RGB_64colors/irgb_bounded RGB_64colors/hsi
	mkdir -p $@
	for I in $$(seq -w 0 ${IMAX}); do \
		compare RGB_64colors/irgb_bounded/RGB_64colors.irgb_bounded.00.$$I.png \
			RGB_64colors/hsi/RGB_64colors.hsi.00.$$I.png \
			$@/RGB_64colors.diff.00.$$I.png; \
		compare RGB_64colors/irgb_bounded/RGB_64colors.irgb_bounded.$$I.00.png \
			RGB_64colors/hsi/RGB_64colors.hsi.$$I.00.png \
			$@/RGB_64colors.diff.$$I.00.png; \
	done
	for I in $$(seq -w 0 ${IMAX2}); do \
		compare RGB_64colors/irgb_bounded/RGB_64colors.irgb_bounded.$$I.$$I.png \
			RGB_64colors/hsi/RGB_64colors.hsi.$$I.$$I.png \
			$@/RGB_64colors.diff.$$I.$$I.png; \
	done

RGB_64colors/%	: RGB_64colors.png ../balance
	mkdir -p $@
	for I in $$(seq -w 0 ${IMAX}); do \
		../balance $* 00 $$I $< $@/RGB_64colors.$*.00.$$I.png; \
		../balance $* $$I 00 $< $@/RGB_64colors.$*.$$I.00.png; \
	done
	for I in $$(seq -w 0 ${IMAX2}); do \
		../balance $* $$I $$I $< $@/RGB_64colors.$*.$$I.$$I.png; \
	done

RGB_64colors_lbl/all	: RGB_64colors_lbl/rgb RGB_64colors_lbl/hsi RGB_64colors_lbl/hsl RGB_64colors_lbl/hsv RGB_64colors_lbl/irgb_bounded RGB_64colors_lbl/irgb_adjusted
	mkdir -p $@
	for I in $$(seq -w 0 ${IMAX}); do \
		montage \
			RGB_64colors_lbl/hsi/RGB_64colors.hsi.00.$$I.tga \
			RGB_64colors_lbl/hsv/RGB_64colors.hsv.00.$$I.tga \
			RGB_64colors_lbl/hsl/RGB_64colors.hsl.00.$$I.tga \
			RGB_64colors_lbl/irgb_bounded/RGB_64colors.irgb_bounded.00.$$I.tga \
			RGB_64colors_lbl/irgb_adjusted/RGB_64colors.irgb_adjusted.00.$$I.tga \
			RGB_64colors_lbl/rgb/RGB_64colors.rgb.00.$$I.tga \
			-geometry +2+2 $@/RGB_64colors.all.00.$$I.tga; \
		montage \
			RGB_64colors_lbl/hsi/RGB_64colors.hsi.$$I.00.tga \
			RGB_64colors_lbl/hsv/RGB_64colors.hsv.$$I.00.tga \
			RGB_64colors_lbl/hsl/RGB_64colors.hsl.$$I.00.tga \
			RGB_64colors_lbl/irgb_bounded/RGB_64colors.irgb_bounded.$$I.00.tga \
			RGB_64colors_lbl/irgb_adjusted/RGB_64colors.irgb_adjusted.$$I.00.tga \
			RGB_64colors_lbl/rgb/RGB_64colors.rgb.$$I.00.tga \
			-geometry +2+2 $@/RGB_64colors.all.$$I.00.tga; \
	done
	for I in $$(seq -w 0 ${IMAX2}); do \
		montage \
			RGB_64colors_lbl/hsi/RGB_64colors.hsi.$$I.$$I.tga \
			RGB_64colors_lbl/hsv/RGB_64colors.hsv.$$I.$$I.tga \
			RGB_64colors_lbl/hsl/RGB_64colors.hsl.$$I.$$I.tga \
			RGB_64colors_lbl/irgb_bounded/RGB_64colors.irgb_bounded.$$I.$$I.tga \
			RGB_64colors_lbl/irgb_adjusted/RGB_64colors.irgb_adjusted.$$I.$$I.tga \
			RGB_64colors_lbl/rgb/RGB_64colors.rgb.$$I.$$I.tga \
			-geometry +2+2 $@/RGB_64colors.all.$$I.$$I.tga; \
	done

RGB_64colors_lbl/%	: RGB_64colors/%
	mkdir -p $@
	for I in $$(seq -w 0 ${IMAX}); do \
		convert $</RGB_64colors.$*.00.$$I.png -pointsize 12 label:"$* 00 $$I" -append $@/RGB_64colors.$*.00.$$I.tga; \
		convert $</RGB_64colors.$*.$$I.00.png -pointsize 12 label:"$* $$I 00" -append $@/RGB_64colors.$*.$$I.00.tga; \
	done
	for I in $$(seq -w 0 ${IMAX2}); do \
		convert $</RGB_64colors.$*.$$I.$$I.png -pointsize 12 label:"$* $$I $$I" -append $@/RGB_64colors.$*.$$I.$$I.tga; \
	done

RGB_64colors.%.00.xx.m4v	: RGB_64colors_lbl/%
	$(RM) -r tmp
	mkdir -p tmp
	for I in $$(seq -w 0 ${IMAX}); do \
		cp -l $</RGB_64colors.$*.00.$$I.tga tmp; \
	done
	for I in $$(seq -w 0 12); do \
		cp -l $</RGB_64colors.$*.00.00.tga tmp/RGB_64colors.$*.00.00.pfx$$I.tga; \
		cp -l $</RGB_64colors.$*.00.${IMAX}.tga tmp/RGB_64colors.$*.00.${IMAX}.sfx$$I.tga; \
	done
	mencoder -ovc lavc -lavcopts vcodec=mpeg4 -mf type=tga:fps=6 -nosound -o $@ mf://tmp/\*

RGB_64colors.%.xx.00.m4v	: RGB_64colors_lbl/%
	$(RM) -r tmp
	mkdir -p tmp
	for I in $$(seq -w 0 ${IMAX}); do \
		cp -l $</RGB_64colors.$*.$$I.00.tga tmp; \
	done
	for I in $$(seq -w 0 12); do \
		cp -l $</RGB_64colors.$*.00.00.tga tmp/RGB_64colors.$*.00.00.pfx$$I.tga; \
		cp -l $</RGB_64colors.$*.${IMAX}.00.tga tmp/RGB_64colors.$*.${IMAX}.00.sfx$$I.tga; \
	done
	mencoder -ovc lavc -lavcopts vcodec=mpeg4 -mf type=tga:fps=6 -nosound -o $@ mf://tmp/\*

RGB_64colors.%.xx.xx.m4v	: RGB_64colors_lbl/%
	$(RM) -r tmp
	mkdir -p tmp
	for I in $$(seq -w 0 ${IMAX2}); do \
		cp -l $</RGB_64colors.$*.$$I.$$I.tga tmp; \
	done
	for I in $$(seq -w 0 12); do \
		cp -l $</RGB_64colors.$*.00.00.tga tmp/RGB_64colors.$*.00.00.pfx$$I.tga; \
		cp -l $</RGB_64colors.$*.${IMAX2}.${IMAX2}.tga tmp/RGB_64colors.$*.${IMAX2}.${IMAX2}.sfx$$I.tga; \
	done
	mencoder -ovc lavc -lavcopts vcodec=mpeg4 -mf type=tga:fps=6 -nosound -o $@ mf://tmp/\*

clean	:
	$(RM) -r tmp
	$(RM) -r RGB_16colors_lbl RGB_64colors_lbl RGB_256colors_lbl
distclean	: clean
	$(RM) -r RGB_16colors RGB_64colors RGB_256colors
	$(RM) -r RGB_16colors.png RGB_64colors.png RGB_256colors.png
	$(RM) -r ${M4V}
