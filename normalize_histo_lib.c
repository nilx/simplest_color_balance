/*
 * Copyright 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file normalize_histo_lib.c
 * @brief normalization routines using an histogram algorithm
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

/**
 * @brief get the min/max of an unsigned char array
 *
 * @param data input array
 * @param size array size
 * @param ptr_min, ptr_max pointers to the returned values, ignored if NULL
 */
static void minmax_u8(const unsigned char *data, size_t size,
                      unsigned char *ptr_min, unsigned char *ptr_max)
{
    const unsigned char *ptr_data, *ptr_end;
    unsigned char min, max;

    /* sanity check */
    if (NULL == data)
    {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }

    /* compute min and max */
    ptr_data = data;
    ptr_end = ptr_data + size;
    min = *ptr_data;
    max = *ptr_data;
    ptr_data++;
    while (ptr_data < ptr_end)
    {
        if (*ptr_data < min)
            min = *ptr_data;
        if (*ptr_data > max)
            max = *ptr_data;
        ptr_data++;
    }

    /* save min and max to the returned pointers if available */
    if (NULL != ptr_min)
        *ptr_min = min;
    if (NULL != ptr_max)
        *ptr_max = max;
    return;
}

/**
 * @brief get quantiles from an unsigned char array such that a given
 * number if pixels is out of this interval
 *
 * This function operates in-place. It computes min (resp. max) such
 * that the number of pixels < min (resp. > max) is inferior or equal
 * to nb_min (resp. nb_max).
 *
 * @param data input/output
 * @param size data array size
 * @param nb_min, nb_max number of pixels to flatten
 * @param ptr_min, ptr_max computed min/max output, ignored if NULL
 */
 void minmax_histo_u8(unsigned char *data, size_t size,
                            size_t nb_min, size_t nb_max,
                            unsigned char *ptr_min, unsigned char *ptr_max)
{
    unsigned char *data_ptr, *data_end;
    /*
     * the histogran must hold all possible "unsigned char" values,
     * including 0
     */
    size_t histo[UCHAR_MAX + 1];
    size_t *histo_ptr, *histo_end;

    /* sanity check */
    if (NULL == data)
    {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }
    if (nb_min + nb_max >= size)
    {
        nb_min = (size - 1) / 2;
        nb_max = (size - 1) / 2;
        fprintf(stderr, "the number of pixels to flatten is too large\n");
        fprintf(stderr, "using (size - 1) / 2\n");
    }

    /* set the histogram to 0 */
    histo_ptr = histo;
    histo_end = histo_ptr + UCHAR_MAX + 1;
    while (histo_ptr < histo_end)
        *histo_ptr++ = 0;
    /* make an histogram */
    data_ptr = data;
    data_end = data_ptr + size;
    while (data_ptr < data_end)
        histo[(size_t) (*data_ptr++)] += 1;

    /* reset the histogram pointer to the second histogram value */
    histo_ptr = histo + 1;
    /* convert the histogram to a cumulative histogram */
    while (histo_ptr < histo_end)
    {
        *histo_ptr += *(histo_ptr - 1);
        histo_ptr++;
    }

    /* get the new min/max */

    if (NULL != ptr_min)
    {
        /* simple forward traversal of the cumulative histogram */
        /* search the first value > flat_nb_min */
        histo_ptr = histo;
        while (histo_ptr < histo_end && *histo_ptr <= nb_min)
            histo_ptr++;
        /* the corresponding histogram value is the current cell indice */
        if(histo_ptr > histo)
           histo_ptr --;
        *ptr_min = histo_ptr - histo;
    }

    if (NULL != ptr_max)
    {
        /* simple backward traversal of the cumulative histogram */
        /* search the first value <= size - nb_max */
        histo_ptr = histo_end - 1;
        while (histo_ptr >= histo && *histo_ptr > (size - nb_max))
            histo_ptr--;
        /*
         * if we are not at the end of the histogram,
         * get to the next cell,
         * ie the last (backward) value > size - flat_nb_max
         */
        if (histo_ptr < histo_end - 1)
            histo_ptr++;
        *ptr_max = histo_ptr - histo;
    }

    return;
}


/**
 * @brief normalize an unsigned char array
 *
 * This function operates in-place. It computes the minimum and
 * maximum values of the data, and rescales the data to the target
 * minimum and maximum, with optionnally flattening some extremal
 * pixels.
 *
 * @param data input/output array
 * @param size array size
 * @param target_min, target_max target min/max values
 * @param flat_nb_min, flat_nb_max number extremal pixels flattened
 *
 * @return data
 */
unsigned char *normalize_histo_u8(unsigned char *data, size_t size,
                                  unsigned char target_min,
                                  unsigned char target_max,
                                  size_t flat_nb_min, size_t flat_nb_max)
{
    unsigned char *data_ptr, *data_end;
    float scale;
    unsigned char target_mid;
    unsigned char min, max;

    /* sanity checks */
    if (NULL == data)
    {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }
    if (flat_nb_min + flat_nb_max >= size)
    {
        flat_nb_min = (size - 1) / 2;
        flat_nb_max = (size - 1) / 2;
        fprintf(stderr, "the number of pixels to flatten is too large\n");
        fprintf(stderr, "using (size - 1) / 2\n");
    }

    /* setup iteration pointers */
    data_ptr = data;
    data_end = data_ptr + size;

    /* target_max == target_min : shortcut */
    if (target_max == target_min)
    {
        while (data_ptr < data_end)
            *data_ptr++ = target_min;
        return data;
    }

    if (0 != flat_nb_min || 0 != flat_nb_max)
        /* get the min/max from the histogram */
        minmax_histo_u8(data, size, flat_nb_min, flat_nb_max, &min, &max);
    else
        /* get the min/max from the data */
        minmax_u8(data, size, &min, &max);

    /* rescale */
    /* max <= min : constant output */
    if (max <= min)
    {
        target_mid = (target_max + target_min) / 2;
        while (data_ptr < data_end)
            *data_ptr++ = target_mid;
    }
    else
    {
        /*
         * build a bounded linear normalization table
         * such that norm(min) = target_min
         *           norm(max) = target_max
         * norm(x) = (x - min) * (t_max - t_min) / (max - min) + t_min
         */
        unsigned char norm[UCHAR_MAX + 1];
        size_t i;

        scale = (float) (target_max - target_min) / (float) (max - min);
        for (i = 0; i < min; i++)
            norm[i] = target_min;
        for (i = min; i < max; i++)
            norm[i] = (unsigned char) ((i - min) * scale + target_min);
        for (i = max; i < UCHAR_MAX + 1; i++)
            norm[i] = target_max;
        /* use the normalization table to transform the data */
        while (data_ptr < data_end)
        {
            *data_ptr = norm[(size_t) (*data_ptr)];
            data_ptr++;
        }
    }
    return data;
}

/**
 * @brief normalize an unsigned char array
 *
 * This function operates in-place. Given the minimum value, it computes the
 * maximum values of the data, and rescales the data to the target
 * minimum and maximum, with optionnally flattening some extremal
 * pixels.
 *
 * @param data input/output array
 * @param size array size
 * @param target_min, target_max target min/max values
 * @param min the minimum of the input array
 * @param flat_nb_max number extremal pixels flattened
 *
 * @return data
 */

unsigned char *normalize_histo_u8_gray(unsigned char *data, size_t size,
                                  unsigned char target_min,
                                  unsigned char target_max,
                                  unsigned char min, unsigned char max )
{

 unsigned char *data_ptr, *data_end;
    float scale;
    unsigned char target_mid;
    


    /* sanity checks */
    if (NULL == data)
    {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }
     data_ptr = data;
    data_end = data_ptr + size;

    /* target_max == target_min : shortcut */
    if (target_max == target_min)
    {
        while (data_ptr < data_end)
            *data_ptr++ = target_min;
        return data;
    }
    
     /* printf("min=%d  max=%d\n", min, max);*/
   if (max <= min)
    {
        target_mid = (target_max + target_min) / 2;
        while (data_ptr < data_end)
            *data_ptr++ = target_mid;
    }
    else
    {
         unsigned char norm[UCHAR_MAX + 1];
        size_t i;

        scale = (float) (target_max - target_min) / (float) (max - min);
        for (i = 0; i < min; i++)
            norm[i] = target_min;
        for (i = min; i < max; i++)
            norm[i] = (unsigned char) ((i - min) * scale + target_min);
        for (i = max; i < UCHAR_MAX + 1; i++)
            norm[i] = target_max;
        /* use the normalization table to transform the data */
        while (data_ptr < data_end)
        {
           *data_ptr = norm[(size_t) (*data_ptr)];
            data_ptr++;
        }
    
     
   }


    return data;
}

/**
*  @brief computes the R G B components of the output image from its gray level 
*
*   Given a color image C=(R, G, B), given its gray level
*
* @f$ gray=(R+G+B)/3 \f$
*
* Given a modified gray image gray1
*
* This function computes an output color image C1=(R1,G1,B1) where each channel is proportional 
* to the input channel and whose gray level is gray1, 
*
* @f$ R1=\frac{gray1}{gray} R    G1=\frac{gray1}{gray} G    B1= \frac{gray1}{gray} B \f$
*
* Note that if the factor gray/gray1 is too big the colors may be saturated to the faces of the RGB cube
*
* @param data_out Output color image
* @param data input color image
* @param gray gray level of the input color image
* @param gray1 modified gray image
* @param dim size of the image
*
* @return data_out
*/

void color_u8(unsigned char *data_out, unsigned char *data, unsigned char *gray, unsigned char *gray1, size_t dim)
{
     
        unsigned char *ptr_red, *ptr_green, *ptr_blue;
        unsigned char *ptr_end,*ptr_gray, *ptr_gray1;
        unsigned char *ptr_in_red, *ptr_in_green, *ptr_in_blue;
        float A,B, rr, gg, bb;
	int ir, ig, ib;
       
 /* sanity check*/
       if (NULL == data_out || NULL == data || NULL== gray || NULL== gray1)
       {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
       }

       
        
        ptr_gray=gray;
        ptr_gray1=gray1;
        ptr_end=ptr_gray+dim;
        ptr_red=data_out;
        ptr_green=data_out+dim;
        ptr_blue=data_out+2*dim;
       
        ptr_in_red=data; ptr_in_green=data+dim; ptr_in_blue=data+2*dim;
        while(ptr_gray< ptr_end){
	   if(*ptr_gray != 0) A=(float) *ptr_gray1/ (float) *ptr_gray;
	   else A=0;
           
            rr=A* (float) (*ptr_in_red); 
            gg=A* (float) (*ptr_in_green);
            bb=A* (float) (*ptr_in_blue);
           if( rr > 255. || gg > 255. || bb > 255.){
               B= (float) *ptr_in_red;
               if((float) *ptr_in_green > B) B= (float) *ptr_in_green;
               if((float) *ptr_in_blue > B) B= (float) *ptr_in_blue;
               A= 255. /B;
               rr=A* (float) (*ptr_in_red);
               gg=A* (float) (*ptr_in_green);
               bb=A* (float) (*ptr_in_blue);
           }
	   ir=(int) (rr+0.5f);
	   ig=(int) (gg+0.5f);
	   ib=(int) (bb+0.5f);
	   *ptr_red=(unsigned char) ir;
	   *ptr_green=(unsigned char) ig;
	   *ptr_blue=(unsigned char) ib;

           ptr_gray++; ptr_gray1++; ptr_red++; ptr_green++; ptr_blue++;
           ptr_in_red++;  ptr_in_green++; ptr_in_blue++;
        }

}





