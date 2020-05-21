#ifndef SIMPLIFIEDETA_H
#define SIMPLIFIEDETA_H

#define c_TADD_BW	16

#define	BITMASK(bitWidth)	( (unsigned int)(-1) >> (sizeof(unsigned int)*8 - (bitWidth)) )
#define GETSIGN(a)			( ((a) >> (c_TADD_BW-1)) & BITMASK(1) )
#define EXTSIGN(a)			( GETSIGN(a) ? ((a) & BITMASK(c_TADD_BW)) | (-1 << c_TADD_BW) : (a) )
#define GETBIT(a, n)		( ((a) >> n) & BITMASK(1) )

#endif
