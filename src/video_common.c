/*      video_common.c
 *
 *      Video stream functions for motion.
 *      Copyright 2000 by Jeroen Vreeken (pe1rxq@amsat.org)
 *                2006 by Krzysztof Blaszkowski (kb@sysmikro.com.pl)
 *                2007 by Angel Carpintero (motiondevelop@gmail.com)
 *      This software is distributed under the GNU public license version 2
 *      See also the file 'COPYING'.
 *
 */
#include "translate.h"
#include "motion.h"
#include "video_common.h"
#include "video_v4l2.h"
#include "video_bktr.h"
#include "jpegutils.h"

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

#define CLAMP(x)  ((x) < 0 ? 0 : ((x) > 255) ? 255 : (x))

typedef struct {
    int is_abs;
    int len;
    int val;
} code_table_t;

/**
 * sonix_decompress_init
 *   pre-calculates a locally stored table for efficient huffman-decoding.
 *
 *   Each entry at index x in the table represents the codeword
 *   present at the MSB of byte x.
 *
 */
static void vid_sonix_decompress_init(code_table_t * table)
{

    int i;
    int is_abs, val, len;

    for (i = 0; i < 256; i++) {
        is_abs = 0;
        val = 0;
        len = 0;
        if ((i & 0x80) == 0) {
            /* code 0 */
            val = 0;
            len = 1;
        } else if ((i & 0xE0) == 0x80) {
            /* code 100 */
            val = +4;
            len = 3;
        } else if ((i & 0xE0) == 0xA0) {
            /* code 101 */
            val = -4;
            len = 3;
        } else if ((i & 0xF0) == 0xD0) {
            /* code 1101 */
            val = +11;
            len = 4;
        } else if ((i & 0xF0) == 0xF0) {
            /* code 1111 */
            val = -11;
            len = 4;
        } else if ((i & 0xF8) == 0xC8) {
            /* code 11001 */
            val = +20;
            len = 5;
        } else if ((i & 0xFC) == 0xC0) {
            /* code 110000 */
            val = -20;
            len = 6;
        } else if ((i & 0xFC) == 0xC4) {
            /* code 110001xx: unknown */
            val = 0;
            len = 8;
        } else if ((i & 0xF0) == 0xE0) {
            /* code 1110xxxx */
            is_abs = 1;
            val = (i & 0x0F) << 4;
            len = 8;
        }
        table[i].is_abs = is_abs;
        table[i].val = val;
        table[i].len = len;
    }
}

/**
 * sonix_decompress
 *      Decompresses an image encoded by a SN9C101 camera controller chip.
 *
 *   IN    width
 *         height
 *         inp     pointer to compressed frame (with header already stripped)
 *   OUT   outp    pointer to decompressed frame
 *
 *         Returns 0 if the operation was successful.
 *         Returns <0 if operation failed.
 *
 */
int vid_sonix_decompress(unsigned char *outp, unsigned char *inp, int width, int height)
{
    int row, col;
    int val;
    int bitpos;
    unsigned char code;
    unsigned char *addr;

    /* Local storage */
    static code_table_t table[256];
    static int init_done = 0;

    if (!init_done) {
        init_done = 1;
        vid_sonix_decompress_init(table);
        /* Do sonix_decompress_init first! */
        //return -1; // so it has been done and now fall through
    }

    bitpos = 0;
    for (row = 0; row < height; row++) {

        col = 0;

        /* First two pixels in first two rows are stored as raw 8-bit. */
        if (row < 2) {
            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
            bitpos += 8;
            *outp++ = code;

            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
            bitpos += 8;
            *outp++ = code;

            col += 2;
        }

        while (col < width) {
            /* Get bitcode from bitstream. */
            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));

            /* Update bit position. */
            bitpos += table[code].len;

            /* Calculate pixel value. */
            val = table[code].val;
            if (!table[code].is_abs) {
                /* Value is relative to top and left pixel. */
                if (col < 2) {
                    /* Left column: relative to top pixel. */
                    val += outp[-2 * width];
                } else if (row < 2) {
                    /* Top row: relative to left pixel. */
                    val += outp[-2];
                } else {
                    /* Main area: average of left pixel and top pixel. */
                    val += (outp[-2] + outp[-2 * width]) / 2;
                }
            }

            /* Store pixel */
            *outp++ = CLAMP(val);
            col++;
        }
    }

    return 0;
}

/**
 * bayer2rgb24
 * BAYER2RGB24 ROUTINE TAKEN FROM:
 *
 * Sonix SN9C10x based webcam basic I/F routines
 * Takafumi Mizuno <taka-qce@ls-a.jp>
 *
 */
void vid_bayer2rgb24(unsigned char *dst, unsigned char *src, long int width, long int height)
{
    long int i;
    unsigned char *rawpt, *scanpt;
    long int size;

    rawpt = src;
    scanpt = dst;
    size = width * height;

    for (i = 0; i < size; i++) {
        if (((i / width) & 1) == 0) {    // %2 changed to & 1
            if ((i & 1) == 0) {
                /* B */
                if ((i > width) && ((i % width) > 0)) {
                    *scanpt++ = *rawpt;     /* B */
                    *scanpt++ = (*(rawpt - 1) + *(rawpt + 1) +
                                *(rawpt + width) + *(rawpt - width)) / 4;    /* G */
                    *scanpt++ = (*(rawpt - width - 1) + *(rawpt - width + 1) +
                                *(rawpt + width - 1) + *(rawpt + width + 1)) / 4;    /* R */
                } else {
                    /* First line or left column. */
                    *scanpt++ = *rawpt;     /* B */
                    *scanpt++ = (*(rawpt + 1) + *(rawpt + width)) / 2;    /* G */
                    *scanpt++ = *(rawpt + width + 1);       /* R */
                }
            } else {
                /* (B)G */
                if ((i > width) && ((i % width) < (width - 1))) {
                    *scanpt++ = (*(rawpt - 1) + *(rawpt + 1)) / 2;  /* B */
                    *scanpt++ = *rawpt;    /* G */
                    *scanpt++ = (*(rawpt + width) + *(rawpt - width)) / 2;  /* R */
                } else {
                    /* First line or right column. */
                    *scanpt++ = *(rawpt - 1);       /* B */
                    *scanpt++ = *rawpt;    /* G */
                    *scanpt++ = *(rawpt + width);   /* R */
                }
            }
        } else {
            if ((i & 1) == 0) {
                /* G(R) */
                if ((i < (width * (height - 1))) && ((i % width) > 0)) {
                    *scanpt++ = (*(rawpt + width) + *(rawpt - width)) / 2;  /* B */
                    *scanpt++ = *rawpt;    /* G */
                    *scanpt++ = (*(rawpt - 1) + *(rawpt + 1)) / 2;  /* R */
                } else {
                    /* Bottom line or left column. */
                    *scanpt++ = *(rawpt - width);   /* B */
                    *scanpt++ = *rawpt;    /* G */
                    *scanpt++ = *(rawpt + 1);       /* R */
                }
            } else {
                /* R */
                if (i < (width * (height - 1)) && ((i % width) < (width - 1))) {
                    *scanpt++ = (*(rawpt - width - 1) + *(rawpt - width + 1) +
                                *(rawpt + width - 1) + *(rawpt + width + 1)) / 4;    /* B */
                    *scanpt++ = (*(rawpt - 1) + *(rawpt + 1) +
                                *(rawpt - width) + *(rawpt + width)) / 4;    /* G */
                    *scanpt++ = *rawpt;     /* R */
                } else {
                    /* Bottom line or right column. */
                    *scanpt++ = *(rawpt - width - 1);       /* B */
                    *scanpt++ = (*(rawpt - 1) + *(rawpt - width)) / 2;    /* G */
                    *scanpt++ = *rawpt;     /* R */
                }
            }
        }
        rawpt++;
    }

}

void vid_yuv422to420p(unsigned char *map, unsigned char *cap_map, int width, int height)
{
    unsigned char *src, *dest, *src2, *dest2;
    int i, j;

    /* Create the Y plane. */
    src = cap_map;
    dest = map;
    for (i = width * height; i > 0; i--) {
        *dest++ = *src;
        src += 2;
    }
    /* Create U and V planes. */
    src = cap_map + 1;
    src2 = cap_map + width * 2 + 1;
    dest = map + width * height;
    dest2 = dest + (width * height) / 4;
    for (i = height / 2; i > 0; i--) {
        for (j = width / 2; j > 0; j--) {
            *dest = ((int) *src + (int) *src2) / 2;
            src += 2;
            src2 += 2;
            dest++;
            *dest2 = ((int) *src + (int) *src2) / 2;
            src += 2;
            src2 += 2;
            dest2++;
        }
        src += width * 2;
        src2 += width * 2;
    }
}

void vid_yuv422pto420p(unsigned char *map, unsigned char *cap_map, int width, int height)
{
    unsigned char *src, *dest, *dest2;
    unsigned char *src_u, *src_u2, *src_v, *src_v2;

    int i, j;
    /*Planar version of 422 */
    /* Create the Y plane. */
    src = cap_map;
    dest = map;
    for (i = width * height; i > 0; i--) {
        *dest++ = *src++;
    }

    /* Create U and V planes. */
    dest = map + width * height;
    dest2 = dest + (width * height) / 4;
    for (i = 0; i< (height / 2); i++) {
        src_u = cap_map + (width * height) + ((i*2) * (width/2));
        src_u2 = src_u  + (width/2);
        src_v = src_u + (width/2 * height);
        src_v2 = src_v  + (width/2);

        for (j = 0; j < (width / 2); j++) {
            *dest = ((int) *src_u + (int) *src_u2) / 2;
            src_u ++;
            src_u2++;
            dest++;

            *dest2 = ((int) *src_v + (int) *src_v2) / 2;
            src_v ++;
            src_v2++;
            dest2++;
        }
    }
}

void vid_uyvyto420p(unsigned char *map, unsigned char *cap_map, int width, int height)
{
    uint8_t *pY = map;
    uint8_t *pU = pY + (width * height);
    uint8_t *pV = pU + (width * height) / 4;
    uint32_t uv_offset = width * 2 * sizeof(uint8_t);
    int ix, jx;

    for (ix = 0; ix < height; ix++) {
        for (jx = 0; jx < width; jx += 2) {
            uint16_t calc;

            if ((ix&1) == 0) {
                calc = *cap_map;
                calc += *(cap_map + uv_offset);
                calc /= 2;
                *pU++ = (uint8_t) calc;
            }

            cap_map++;
            *pY++ = *cap_map++;

            if ((ix&1) == 0) {
                calc = *cap_map;
                calc += *(cap_map + uv_offset);
                calc /= 2;
                *pV++ = (uint8_t) calc;
            }

            cap_map++;
            *pY++ = *cap_map++;
        }
    }
}

void vid_rgb24toyuv420p(unsigned char *map, unsigned char *cap_map, int width, int height)
{
    unsigned char *y, *u, *v;
    unsigned char *r, *g, *b;
    int i, loop;

    r = cap_map;
    g = r + 1;
    b = g + 1;

    y = map;
    u = y + width * height;
    v = u + (width * height) / 4;
    memset(u, 0, width * height / 4);
    memset(v, 0, width * height / 4);

    for (loop = 0; loop < height; loop++) {
        for (i = 0; i < width; i += 2) {
            *y++ = (9796 ** r + 19235 ** g + 3736 ** b) >> 15;
            *u += ((-4784 ** r - 9437 ** g + 14221 ** b) >> 17) + 32;
            *v += ((20218 ** r - 16941 ** g - 3277 ** b) >> 17) + 32;
            r += 3;
            g += 3;
            b += 3;
            *y++ = (9796 ** r + 19235 ** g + 3736 ** b) >> 15;
            *u += ((-4784 ** r - 9437 ** g + 14221 ** b) >> 17) + 32;
            *v += ((20218 ** r - 16941 ** g - 3277 ** b) >> 17) + 32;
            r += 3;
            g += 3;
            b += 3;
            u++;
            v++;
        }

        if ((loop & 1) == 0) {
            u -= width / 2;
            v -= width / 2;
        }
    }
}

/**
 * mjpegtoyuv420p
 *
 * Return values
 *  -1 on fatal error
 *  0  on success
 *  2  if jpeg lib threw a "corrupt jpeg data" warning.
 *     in this case, "a damaged output image is likely."
 */
int vid_mjpegtoyuv420p(unsigned char *map, unsigned char *cap_map, int width, int height, unsigned int size)
{
    unsigned char *ptr_buffer;
    size_t soi_pos = 0;
    int ret = 0;

    ptr_buffer = memmem(cap_map, size, "\xff\xd8", 2);
    if (ptr_buffer == NULL) {
        MOTION_LOG(CRT, TYPE_VIDEO, NO_ERRNO,_("Corrupt image ... continue"));
        return 1;
    }
    /**
     Some cameras are sending multiple SOIs in the buffer.
     Move the pointer to the last SOI in the buffer and proceed.
    */
    while (ptr_buffer != NULL && ((size - soi_pos - 1) > 2) ){
        soi_pos = ptr_buffer - cap_map;
        ptr_buffer = memmem(cap_map + soi_pos + 1, size - soi_pos - 1, "\xff\xd8", 2);
    }

    if (soi_pos != 0){
        MOTION_LOG(INF, TYPE_VIDEO, NO_ERRNO,_("SOI position adjusted by %d bytes."), soi_pos);
    }

    memmove(cap_map, cap_map + soi_pos, size - soi_pos);
    size -= soi_pos;

    ret = jpgutl_decode_jpeg(cap_map,size, width, height, map);

    if (ret == -1) {
        MOTION_LOG(CRT, TYPE_VIDEO, NO_ERRNO,_("Corrupt image ... continue"));
        ret = 1;
    }
    return ret;
}

void vid_y10torgb24(unsigned char *map, unsigned char *cap_map, int width, int height, int shift)
{
    /* Source code: raw2rgbpnm project */
    /* url: http://salottisipuli.retiisi.org.uk/cgi-bin/gitweb.cgi?p=~sailus/raw2rgbpnm.git;a=summary */

    /* bpp - bits per pixel */
    /* bpp: 'Pixels are stored in 16-bit words with unused high bits padded with 0' */
    /* url: https://linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-Y12.html */
    /* url: https://linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-Y10.html */

    int src_size[2] = {width,height};
    int bpp = 16;
    unsigned int src_stride = (src_size[0] * bpp) / 8;
    unsigned int rgb_stride = src_size[0] * 3;
    int a = 0;
    int src_x = 0, src_y = 0;
    int dst_x = 0, dst_y = 0;

    for (src_y = 0, dst_y = 0; dst_y < src_size[1]; src_y++, dst_y++) {
        for (src_x = 0, dst_x = 0; dst_x < src_size[0]; src_x++, dst_x++) {
            a = (cap_map[src_y*src_stride + src_x*2+0] |
                (cap_map[src_y*src_stride + src_x*2+1] << 8)) >> shift;
            map[dst_y*rgb_stride+3*dst_x+0] = a;
            map[dst_y*rgb_stride+3*dst_x+1] = a;
            map[dst_y*rgb_stride+3*dst_x+2] = a;
        }
    }
}

void vid_greytoyuv420p(unsigned char *map, unsigned char *cap_map, int width, int height)
{

    memcpy(map, cap_map, (width*height));
    memset(map+(width*height), 128, (width * height) / 2);

}

/* vid_parms_free
 * Free all the memory associated with the parameter control array.
*/
static void vid_parms_free(struct context *cnt)
{
    int indx_parm;

    for (indx_parm=0; indx_parm<cnt->vdev->usrctrl_count; indx_parm++)
    {
        free(cnt->vdev->usrctrl_array[indx_parm].ctrl_name);
        cnt->vdev->usrctrl_array[indx_parm].ctrl_name = NULL;
    }

    if (cnt->vdev->usrctrl_array != NULL) {
      free(cnt->vdev->usrctrl_array);
      cnt->vdev->usrctrl_array = NULL;
    }

    cnt->vdev->usrctrl_count = 0;

}

/* vid_parms_add
 * Add the parsed out parameter and value to the control array.
*/
static void vid_parms_add(struct context *cnt, const char *parm_nm, const char *parm_val)
{
    cnt->vdev->usrctrl_count++;

    if (cnt->vdev->usrctrl_count == 1) {
        cnt->vdev->usrctrl_array =(struct vdev_usrctrl_ctx *) mymalloc(sizeof(struct vdev_usrctrl_ctx));
    } else {
        cnt->vdev->usrctrl_array =(struct vdev_usrctrl_ctx *)realloc(cnt->vdev->usrctrl_array
            , sizeof(struct vdev_usrctrl_ctx)*cnt->vdev->usrctrl_count);
    }

    if (parm_nm != NULL) {
        cnt->vdev->usrctrl_array[cnt->vdev->usrctrl_count-1].ctrl_name =(char*)mymalloc(strlen(parm_nm)+1);
        sprintf(cnt->vdev->usrctrl_array[cnt->vdev->usrctrl_count-1].ctrl_name,"%s",parm_nm);
    } else {
        cnt->vdev->usrctrl_array[cnt->vdev->usrctrl_count-1].ctrl_name = NULL;
    }

    cnt->vdev->usrctrl_array[cnt->vdev->usrctrl_count-1].ctrl_value = atoi(parm_val);

    MOTION_LOG(INF, TYPE_VIDEO, NO_ERRNO,_("Parsed: >%s< >%d<")
        ,cnt->vdev->usrctrl_array[cnt->vdev->usrctrl_count-1].ctrl_name
        ,cnt->vdev->usrctrl_array[cnt->vdev->usrctrl_count-1].ctrl_value);

}

/* vid_parms_set
 * Extract out of the string the parameter name and values at the location
 * specified.
*/
static void vid_parms_set(struct context *cnt, char *parms
        ,int indxnm_st, int indxnm_en, int indxvl_st, int indxvl_en)
{
    char *parm_nm, *parm_vl;
    int retcd, chksz;

    if ((indxnm_en != 0) &&
        (indxvl_st != 0) &&
        ((indxnm_en - indxnm_st) > 0) &&
        ((indxvl_en - indxvl_st) > 0))
    {
        parm_nm = mymalloc(PATH_MAX);
        parm_vl = mymalloc(PATH_MAX);

        chksz = indxnm_en - indxnm_st + 1;
        retcd = snprintf(parm_nm, chksz, "%s", parms + indxnm_st);
        if (retcd < 0) {
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("Error parsing parm_nm controls: %s"), parms);
        }

        chksz = indxvl_en - indxvl_st + 1;
        retcd = snprintf(parm_vl, chksz, "%s", parms + indxvl_st);
        if (retcd < 0) {
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("Error parsing parm_vl controls: %s"), parms);
        }

        util_trim(parm_nm);
        util_trim(parm_vl);

        vid_parms_add(cnt, parm_nm, parm_vl);

        free(parm_nm);
        free(parm_vl);
    }

}

/* vid_parms_next
 * Remove the parameter parsed out in previous steps from the parms string
 * and set up the string for parsing out the next parameter.
*/
static void vid_parms_next(char *parms,int indxnm_st, int indxvl_en)
{
    char *parm_tmp;
    int retcd;

    parm_tmp = mymalloc(PATH_MAX);
    retcd = snprintf(parm_tmp, PATH_MAX, "%s", parms);
    if (retcd < 0) {
        MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("Error setting temp: %s"), parms);
    }

    if (indxnm_st == 0) {
        if ((size_t)(indxvl_en + 1) >strlen(parms)) {
            parms[0]='\0';
        } else {
            retcd = snprintf(parms, strlen(parm_tmp) - indxvl_en + 1
                , "%s", parm_tmp+indxvl_en+1);
        }
    } else {
        if ((size_t)(indxvl_en + 1) > strlen(parms) ) {
            retcd = snprintf(parms, indxnm_st - 1, "%s", parm_tmp);
        } else {
            retcd = snprintf(parms, PATH_MAX, "%.*s%.*s"
                , indxnm_st - 1, parm_tmp
                , (int)(strlen(parm_tmp) - indxvl_en)
                , parm_tmp + indxvl_en);
        }
    }
    if (retcd < 0) {
        MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("Error reparsing controls: %s"), parms);
    }

    free(parm_tmp);

}

/* vid_parms_parse_qte
 * Split out the parameters that have quotes around the name.
*/
static void vid_parms_parse_qte(struct context *cnt, char *parms)
{
    int indxnm_st, indxnm_en, indxvl_st, indxvl_en;

    while (strstr(parms,"\"") != NULL)
    {
        indxnm_st = 0;
        indxnm_en = 0;
        indxvl_st = 0;
        indxvl_en = strlen(parms);

        indxnm_st = strstr(parms,"\"") - parms + 1;
        if (strstr(parms + indxnm_st,"\"") != NULL) {
            indxnm_en = strstr(parms + indxnm_st,"\"") - parms;
            if (strstr(parms + indxnm_en + 1,"=") != NULL) {
                indxvl_st = strstr(parms + indxnm_en + 1,"=") - parms + 1;
            }
            if (strstr(parms + indxvl_st + 1,",") != NULL) {
                indxvl_en = strstr(parms + indxvl_st + 1,",") - parms;
            }
        }

        vid_parms_set(cnt, parms, indxnm_st, indxnm_en, indxvl_st, indxvl_en);
        vid_parms_next(parms, indxnm_st, indxvl_en);
    }
}

/* vid_parms_parse_comma
 * Split out the parameters between the commas.
*/
static void vid_parms_parse_comma(struct context *cnt, char *parms)
{
    int indxnm_st, indxnm_en, indxvl_st, indxvl_en;

    while (strstr(parms,",") != NULL)
    {
        indxnm_st = 0;
        indxnm_en = 0;
        indxvl_st = 0;
        indxvl_en = strstr(parms, ",") - parms;

        if (strstr(parms, "=") != NULL) {
            indxnm_en = strstr(parms,"=") - parms;
            if ((size_t)indxnm_en < strlen(parms)) {
                indxvl_st = indxnm_en + 1;
            }
        }
        vid_parms_set(cnt, parms, indxnm_st, indxnm_en, indxvl_st, indxvl_en);
        vid_parms_next(parms, indxnm_st, indxvl_en);
    }

}

/* vid_parms_parse
 * Parse the vid_control_params into an array.
*/
int vid_parms_parse(struct context *cnt)
{
    /* Parse through the configuration option to get values
     * The values are separated by commas but may also have
     * double quotes around the names which include a comma.
     * Examples:
     * vid_control_params ID01234= 1, ID23456=2
     * vid_control_params "Brightness, auto" = 1, ID23456=2
     * vid_control_params ID23456=2, "Brightness, auto" = 1,ID2222=5
     */

    int retcd, indxnm_st, indxnm_en, indxvl_st, indxvl_en;
    char *parms;

    if (!cnt->vdev->update_parms) return 0;

    vid_parms_free(cnt);
    parms = NULL;

    if (cnt->conf.vid_control_params != NULL) {
        MOTION_LOG(INF, TYPE_VIDEO, NO_ERRNO
            ,_("Parsing controls: %s"), cnt->conf.vid_control_params);

        parms = mymalloc(PATH_MAX);

        retcd = snprintf(parms, PATH_MAX, "%s", cnt->conf.vid_control_params);
        if ((retcd < 0) || (retcd > PATH_MAX)) {
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO
                ,_("Error parsing controls: %s"), cnt->conf.vid_control_params);
            free(parms);
            return 0;
        }

        vid_parms_parse_qte(cnt, parms);

        vid_parms_parse_comma(cnt, parms);

        if (strlen(parms) != 0) {
            indxnm_st = 0;
            indxnm_en = 0;
            indxvl_st = 0;
            indxvl_en = strlen(parms);
            if (strstr(parms + 1,"=") != NULL) {
                indxnm_en = strstr(parms + 1,"=") - parms;
                if ((size_t)indxnm_en < strlen(parms)){
                    indxvl_st = indxnm_en + 1;
                }
            }

            vid_parms_set(cnt, parms, indxnm_st, indxnm_en, indxvl_st, indxvl_en);
        }
        free(parms);
    }

    cnt->vdev->update_parms = FALSE;

    return 0;

}


void vid_mutex_init(void)
{
    v4l2_mutex_init();
    bktr_mutex_init();
}

void vid_mutex_destroy(void)
{
    v4l2_mutex_destroy();
    bktr_mutex_destroy();
}

void vid_close(struct context *cnt) {

#ifdef HAVE_MMAL
    if (cnt->mmalcam) {
        MOTION_LOG(INF, TYPE_VIDEO, NO_ERRNO,_("calling mmalcam_cleanup"));
        mmalcam_cleanup(cnt->mmalcam);
        cnt->mmalcam = NULL;
        return;
    }
#endif

    if (cnt->netcam) {
        MOTION_LOG(INF, TYPE_VIDEO, NO_ERRNO,_("calling netcam_cleanup"));
        netcam_cleanup(cnt->netcam, 0);
        cnt->netcam = NULL;
        return;
    }

    if (cnt->rtsp) {
        /* This also cleans up high resolution */
        MOTION_LOG(INF, TYPE_VIDEO, NO_ERRNO,_("calling netcam_rtsp_cleanup"));
        netcam_rtsp_cleanup(cnt, 0);
        return;
    }

    if (cnt->camera_type == CAMERA_TYPE_V4L2) {
        MOTION_LOG(NTC, TYPE_VIDEO, NO_ERRNO,_("Cleaning up V4L2 device"));
        v4l2_cleanup(cnt);
        return;
    }

    if (cnt->camera_type == CAMERA_TYPE_BKTR) {
        MOTION_LOG(NTC, TYPE_VIDEO, NO_ERRNO,_("Cleaning up BKTR device"));
        bktr_cleanup(cnt);
        return;
    }

    MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("No Camera device cleanup (MMAL, Netcam, V4L2, BKTR)"));
    return;

}

/**
 * vid_start
 *
 * vid_start setup the capture device. This will be either a V4L device or a netcam.
 * The function does the following:
 * - If the camera is a netcam - netcam_start is called and function returns
 * - Width and height are checked for valid value (multiple of 8)
 * - Copy the config height and width to the imgs struct. Note that height and width are
 *   only copied to the from the conf struct to the imgs struct during program startup
 *   The width and height can no later be changed via http remote control as this would
 *   require major re-memory allocations of all image buffers.
 *
 * - if the camera is V4L2 v4l2_start is called
 *
 * Parameters:
 *     cnt        Pointer to the context for this thread
 *
 * Returns
 *     device number
 *     -1 if failed to open device.
 *     -3 image dimensions are not modulo 8
 */
int vid_start(struct context *cnt) {
    int dev = -1;

#ifdef HAVE_MMAL
    if (cnt->camera_type == CAMERA_TYPE_MMAL) {
        MOTION_LOG(NTC, TYPE_VIDEO, NO_ERRNO,_("Opening MMAL cam"));
        dev = mmalcam_start(cnt);
        if (dev < 0) {
            mmalcam_cleanup(cnt->mmalcam);
            cnt->mmalcam = NULL;
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("MMAL cam failed to open"));
        }
        return dev;
    }
#endif

    if (cnt->camera_type == CAMERA_TYPE_NETCAM) {
        MOTION_LOG(NTC, TYPE_VIDEO, NO_ERRNO,_("Opening Netcam"));
        dev = netcam_start(cnt);
        if (dev < 0) {
            netcam_cleanup(cnt->netcam, 1);
            cnt->netcam = NULL;
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("Netcam failed to open"));
        }
        return dev;
    }

    if (cnt->camera_type == CAMERA_TYPE_RTSP) {
        MOTION_LOG(NTC, TYPE_VIDEO, NO_ERRNO,_("Opening Netcam RTSP"));
        dev = netcam_rtsp_setup(cnt);
        if (dev < 0) {
            netcam_rtsp_cleanup(cnt, 1);
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("Netcam RTSP failed to open"));
        }
        return dev;
    }

    if (cnt->camera_type == CAMERA_TYPE_V4L2) {
        MOTION_LOG(NTC, TYPE_VIDEO, NO_ERRNO,_("Opening V4L2 device"));
        dev = v4l2_start(cnt);
        if (dev < 0) {
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("V4L2 device failed to open"));
        }
        return dev;
    }

    if (cnt->camera_type == CAMERA_TYPE_BKTR) {
        MOTION_LOG(NTC, TYPE_VIDEO, NO_ERRNO,_("Opening BKTR device"));
        dev = bktr_start(cnt);
        if (dev < 0) {
            MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO,_("BKTR device failed to open"));
        }
        return dev;
    }

    MOTION_LOG(ERR, TYPE_VIDEO, NO_ERRNO
        ,_("No Camera device specified (MMAL, Netcam, V4L2, BKTR)"));
    return dev;

}

/**
 * vid_next
 *
 * vid_next fetches a video frame from a either v4l device or netcam
 *
 * Parameters:
 *     cnt        Pointer to the context for this thread
 *     map        Pointer to the buffer in which the function puts the new image
 *
 * Global variable
 *     viddevs    The viddevs struct is "global" within the context of video.c
 *                and used in functions vid_*.
 * Returns
 *     0                        Success
 *    -1                        Fatal V4L error
 *    -2                        Fatal Netcam error
 *    Positive numbers...
 *    with bit 0 set            Non fatal V4L error (copy grey image and discard this image)
 *    with bit 1 set            Non fatal Netcam error
 */
int vid_next(struct context *cnt, struct image_data *img_data){

#ifdef HAVE_MMAL
     if (cnt->camera_type == CAMERA_TYPE_MMAL) {
        if (cnt->mmalcam == NULL) {
            return NETCAM_GENERAL_ERROR;
        }
        return mmalcam_next(cnt, img_data);
    }
#endif

    if (cnt->camera_type == CAMERA_TYPE_NETCAM) {
        if (cnt->video_dev == -1)
            return NETCAM_GENERAL_ERROR;

        return netcam_next(cnt, img_data);
    }

    if (cnt->camera_type == CAMERA_TYPE_RTSP) {
        if (cnt->video_dev == -1)
            return NETCAM_GENERAL_ERROR;

        return netcam_rtsp_next(cnt, img_data);
    }

    if (cnt->camera_type == CAMERA_TYPE_V4L2) {
        return v4l2_next(cnt, img_data);
   }

    if (cnt->camera_type == CAMERA_TYPE_BKTR) {
        return bktr_next(cnt, img_data);
    }

    return -2;
}
