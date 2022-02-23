/* XenonJS : Lexer : Helper
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_LEXER_HELPER_H
#define XJS_LEXER_HELPER_H

#include "xjs_dt.h"

/* Whitespace:
 * TODO: Any other Unicode "Seperator, space" codepoint */
#define XJS_CHAR_IS_WS(ch) \
    (((ch) == (xjs_char_t)('\t')) || \
     ((ch) == (xjs_char_t)('\v')) || \
     ((ch) == (xjs_char_t)(' ')) || \
     ((ch) == (xjs_char_t)('\f')) || \
     ((ch) == (xjs_char_t)('\r')) || \
     ((ch) == (xjs_char_t)('\n')) || \
     ((ch) == (xjs_char_t)(0xA0)) || \
     ((ch) == (xjs_char_t)(0xFEFF)))

/* Line Terminator */
#define XJS_CHAR_IS_LT(ch) \
    (((ch) == (xjs_char_t)('\r')) || \
     ((ch) == (xjs_char_t)('\n')) || \
     ((ch) == (xjs_char_t)(0x2028)) || \
     ((ch) == (xjs_char_t)(0x2029)))

/* Digit */
#define XJS_CHAR_IS_DIGIT_BIN(ch) \
    (((xjs_char_t)('0') == (ch)) || ((ch) == (xjs_char_t)('1')))

#define XJS_CHAR_IS_DIGIT_OCT(ch) \
    (((xjs_char_t)('0') <= (ch)) && ((ch) <= (xjs_char_t)('7')))

#define XJS_CHAR_IS_DIGIT_DEC(ch) \
    (((xjs_char_t)('0') <= (ch)) && ((ch) <= (xjs_char_t)('9')))

#define XJS_CHAR_IS_DIGIT_HEX(ch) \
    (XJS_CHAR_IS_DIGIT_DEC(ch) || \
     (((xjs_char_t)('a') <= (ch)) && ((ch) <= (xjs_char_t)('f'))) || \
     (((xjs_char_t)('A') <= (ch)) && ((ch) <= (xjs_char_t)('F'))))

/* TODO: Add more Unicode chars
 * We found some useful resource about how to extract those Unicode characters, unluckily, the table's got too huge,
 * no plan to do this job currently. 
 * https://stackoverflow.com/questions/43150498/how-to-get-all-unicode-characters-from-specific-categories/43163378#43163378 */

/* Unicode ID Start */
#define XJS_CHAR_IS_UNICODE_ID_START(ch) \
    (((ch) == '$') || ((ch) == '_') || \
     (((xjs_char_t)('a') <= (ch)) && ((ch) <= (xjs_char_t)('z'))) || \
     (((xjs_char_t)('A') <= (ch)) && ((ch) <= (xjs_char_t)('Z'))))

/* Unicode ID Continue */
#define XJS_CHAR_IS_UNICODE_ID_CONTINUE(ch) \
    (((ch) == '$') || ((ch) == '_') || \
     (((xjs_char_t)('a') <= (ch)) && ((ch) <= (xjs_char_t)('z'))) || \
     (((xjs_char_t)('A') <= (ch)) && ((ch) <= (xjs_char_t)('Z'))) || \
     (((xjs_char_t)('0') <= (ch)) && ((ch) <= (xjs_char_t)('9'))))

#endif

