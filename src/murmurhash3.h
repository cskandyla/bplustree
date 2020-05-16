#ifndef MURMURHASH3_H
#define MURMURHASH3_H
#include <stdint.h>
uint32_t  MurmurHash3_32(void *key,int len,uint32_t seed);

static inline uint32_t getblock(uint8_t *p,int i);
static inline uint32_t rotl(uint32_t x,uint8_t r);
static inline uint32_t fmix(uint32_t h);


#endif
