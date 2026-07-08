#ifndef __CHINESE_CHARS_H
#define __CHINESE_CHARS_H

/*
 * chinese_chars.h
 * ----------------
 * Stores Chinese character strings as GBK byte arrays.
 * Each variable is named by pinyin for readability.
 * Usage: pass the array pointer to LCD_ShowChinese().
 *
 * GBK encoding reference:
 *   ni   (you)     0xC4 0xE3
 *   hao  (good)    0xBA 0xC3
 *   shi  (world1)  0xCA 0xBE
 *   jie  (world2)  0xBD 0xE7
 *   wen  (temp1)   0xCE 0xC2
 *   du   (temp2)   0xB6 0xC8
 */

/* "ni hao" = two characters: ni(you) + hao(good) */
static const unsigned char str_nihao[]    = {0xC4,0xE3, 0xBA,0xC3, 0x00};

/* "shi jie" = two characters: shi(world part1) + jie(world part2) */
static const unsigned char str_shijie[]   = {0xCA,0xBE, 0xBD,0xE7, 0x00};

/* "ni hao shi jie" = four characters */
static const unsigned char str_nihaoshijie[] = {0xC4,0xE3, 0xBA,0xC3, 0xCA,0xBE, 0xBD,0xE7, 0x00};

/* "wen du" = two characters: wen(temperature1) + du(temperature2) */
static const unsigned char str_wendu[]    = {0xCE,0xC2, 0xB6,0xC8, 0x00};

#endif /* __CHINESE_CHARS_H */
