#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED
#include <cmath>

typedef long long SIMHASH_TYPE;

const int ERROR_OPENFILE = -1;
const int OK_READFILE = 0;

const int LINE_LENGTH = 10240;

const int HAMMINGDIST = 3;

const int SIMHASHBITS = 64;

const int KGRAM = 6;
const int BASE = 12;
//const SIMHASH_TYPE MODNUM = ((unsigned SIMHASH_TYPE)1 << (SIMHASHBITS-1))- 1;
//const SIMHASH_TYPE MODNUM = 9223372036854775783L; //64位长整形的最大质数
const SIMHASH_TYPE MODNUM = (((unsigned SIMHASH_TYPE)1 << (SIMHASHBITS-1))- 1)/BASE; //不能取太大的值，否则在计算KRHash时会发生溢出，导致结果不准确
const int WINDOWSIZE = 10;

const double TFGATE = 50;
const int STFGATE = std::ceil(0.8 * KGRAM); //KGRMA中停用词的比例小于80%

#endif // CONSTANTS_H_INCLUDED
