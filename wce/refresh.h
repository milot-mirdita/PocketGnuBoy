#ifndef __REFRESH_H__
#define __REFRESH_H__

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char *screenvram;
extern int screenpitchx;
extern int screenpitchy;
extern int screenmode;

INLINE void refresh_2(un16 *dest, byte *src, un16 *pal, int cnt)
{
        int i;
        un16 *pixel, *pixel2;

        if (screenmode == 1) { // 240 x 216
                cnt >>= 2;
                pixel = (un16 *) (screenvram + screenpitchy * ((scan.l * 3) >> 1));
                if (scan.l & 0x1) {
                        pixel2 = pixel + (screenpitchy >> 1);
                        for (i = 0; i < cnt; i++) {
                                *(pixel) = *(pixel2) = pal[*(src)];
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;

                                *(pixel) = *(pixel2) = pal[*(src++)];
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;

                                *(pixel) = *(pixel2) = pal[*(src++)];
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;

                                *(pixel) = *(pixel2) = pal[*(src++)];
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;

                                *(pixel) = *(pixel2) = pal[*(src)];
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;

                                *(pixel) = *(pixel2) = pal[*(src++)];
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;
                        }
                }
                else {
                        for (i = 0; i < cnt; i++) {
                                *(pixel) = pal[*(src)];
                                (pixel) += screenpitchx;
                                *(pixel) = pal[*(src++)];
                                (pixel) += screenpitchx;
                                *(pixel) = pal[*(src++)];
                                (pixel) += screenpitchx;

                                *(pixel) = pal[*(src++)];
                                (pixel) += screenpitchx;
                                *(pixel) = pal[*(src)];
                                (pixel) += screenpitchx;
                                *(pixel) = pal[*(src++)];
                                (pixel) += screenpitchx;
                        }
                }
        } else if (screenmode == 2) { // 240 x 216 (half antialiasing)
                static un16 p0, p1, p01;
                cnt >>= 1;
                pixel = (un16 *) (screenvram + screenpitchy * ((scan.l * 3) >> 1));
                for (i = 0; i < cnt; i++) {
                        /* Plot three pixels
                         * [p0] <p01> [p1]
                         * where p01 is calculated from the average of
                         * p0 and p1
                         */

                        p0 = pal[*(src++)];
                        p1 = pal[*(src++)];


                        *(pixel) = p0;
                        (pixel) += screenpitchx;
                        *(pixel) =      ((((p0 & 0xf800) + (p1 & 0xf800)) >> 1) & 0xf800) |
                                                ((((p0 & 0x07e0) + (p1 & 0x07e0)) >> 1) & 0x07e0) |
                                                (((p0 & 0x001f) + (p1 & 0x001f)) >> 1);
                        (pixel) += screenpitchx;
                        *(pixel) = p1;
                        (pixel) += screenpitchx;
                }

                if (scan.l & 0x1) {
                        pixel = (un16 *) (screenvram + screenpitchy * ((scan.l * 3) >> 1));
                        memcpy(pixel + (screenpitchy >> 1), pixel, 240 * 2);
                }
        } else if (screenmode == 3) { // 240 x 216 (antialiasing)
                /* "mean"/"average" effect */
                cnt >>= 1;
                pixel = (un16 *) (screenvram + screenpitchy * ((scan.l * 3) >> 1));
                if (scan.l & 0x1) {
                        un16 p00, p01, p02;
                        un16 p10, p11, p12;
                        un16 p20, p21, p22;
                        un16 r20, g20, b20;
                        un16 r22, g22, b22;
                        for (i = 0; i < cnt; i++) {
                                /* plot six pixels
                                 * [p00] <p01> [p02]
                                 * >p10< >p11< >p12<
                                 * [p20] >p21< [p22]
                                 */

                                un16 *pixel0 = pixel - (screenpitchy >> 1);
                                un16 *pixel2 = pixel + (screenpitchy >> 1);

                                p00 = *pixel0;
                                p01 = *(pixel0 + screenpitchx);
                                p02 = *(pixel0 + 2*screenpitchx);

                                p20 = pal[*(src++)];
                                p22 = pal[*(src++)];

                                r20 = p20 & 0xf800;
                                g20 = p20 & 0x07e0;
                                b20 = p20 & 0x001f;

                                r22 = p22 & 0xf800;
                                g22 = p22 & 0x07e0;
                                b22 = p22 & 0x001f;

                                p21 =   (((r20 + r22) >> 1) & 0xf800) |
                                                (((g20 + g22) >> 1) & 0x07e0) |
                                                ((b20 + b22) >> 1);

                                p10 =   (((r20 + (p00 & 0xf800)) >> 1) & 0xf800) |
                                                (((g20 + (p00 & 0x07e0)) >> 1) & 0x07e0) |
                                                ((b20 + (p00 & 0x001f)) >> 1);

                                p12=    ((((p02 & 0xf800) + r22) >> 1) & 0xf800) |
                                                ((((p02 & 0x07e0) + g22) >> 1) & 0x07e0) |
                                                (((p02 & 0x001f) + b22) >> 1);

                                p11=    ((((p01 & 0xf800) + (p21 & 0xf800)) >> 1) & 0xf800) |
                                                ((((p01 & 0x07e0) + (p21 & 0x07e0)) >> 1) & 0x07e0) |
                                                (((p01 & 0x001f) + (p21 & 0x001f)) >> 1);

                                *(pixel) = p10;
                                *(pixel2) = p20;
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;

                                *(pixel) = p11;
                                *(pixel2) = p21;
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;

                                *(pixel) = p12;
                                *(pixel2) = p22;
                                (pixel) += screenpitchx;
                                (pixel2) += screenpitchx;
                        }
                }
                else {
                        static un16 p0, p1, p01;
                        for (i = 0; i < cnt; i++) {
                                /* Plot three pixels
                                 * [p0] <p01> [p1]
                                 * where p01 is calculated from the average of
                                 * p0 and p1
                                 */

                                p0 = pal[*(src++)];
                                p1 = pal[*(src++)];


                                *(pixel) = p0;
                                (pixel) += screenpitchx;
                                *(pixel) =      ((((p0 & 0xf800) + (p1 & 0xf800)) >> 1) & 0xf800) |
                                                        ((((p0 & 0x07e0) + (p1 & 0x07e0)) >> 1) & 0x07e0) |
                                                        (((p0 & 0x001f) + (p1 & 0x001f)) >> 1);
                                (pixel) += screenpitchx;
                                *(pixel) = p1;
                                (pixel) += screenpitchx;
                        }
                }
        }
        else { // 160 x 144
                pixel = (un16 *) (screenvram + screenpitchy * scan.l);
                for (i = 0; i < cnt; i++) {
                        *(pixel) = pal[*(src++)];
                        (pixel) += screenpitchx;
                }
        }
}

#ifdef __cplusplus
}
#endif

#endif // __REFRESH_H__
