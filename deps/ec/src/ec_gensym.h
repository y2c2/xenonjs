/* Enhanced C : Symbol Generator
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_GENSYM_H
#define EC_GENSYM_H

#define EC_GENSYM2(x, y) x##y
#define EC_GENSYM1(x, y) EC_GENSYM2(x, y)
#define ec_gensym(x) EC_GENSYM1(x, __COUNTER__)

#endif
