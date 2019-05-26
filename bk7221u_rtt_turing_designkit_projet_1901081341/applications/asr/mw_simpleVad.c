/*
 simpleVad for AGC
*/
#include <stdlib.h>
//#include <android/log.h>

/* the th of ZCR*/
#define  ZCR_MIN( a )	  ( 2.0f * (a) / 80)
#define  ZCR_MAX( a )	  ( 70.0f * (a) / 80)

/* the th of EZCR*/
#define  EZCR_TH( a )	  ( 1.0f *80 / (a) )

#define  MAX_NOISE        (3000)
#define  MIN_NOISE        (200)
#define  BRUST            15
#define  HANG             50

#define  VAD_SIGN(x)  (((x)>=0) ? 1 : -1)


/** SimpleVad state. */
typedef struct SimpleVadState_T{
   /*****vqe vad************/
   int maxlevel;	
   int minlevel;
   int zcr;
   int vadflag;
   int burst_count;
   int hang_count;
   float noiseLevel;
   float envelope;
   float avglevel;
   float ezcr;
   int vqeMode;
   int vad_flag;
} SimpleVadState;

int SimpleVadCreate(SimpleVadState **handle)
{
	  SimpleVadState* self = NULL;

	  if (handle == NULL) {
	    return -1;
	  }

	  self  = (SimpleVadState *) sdram_malloc(sizeof(SimpleVadState));

	  if (self == NULL) {
	    return -1;
	  }

	  *handle = (SimpleVadState *) self;

	  self->avglevel = 0.0f;
	  self->maxlevel = 0;
	  self->minlevel = 0;
	  self->zcr = 0;
	  self->ezcr = 0;
	  self->burst_count = 0;
	  self->hang_count = 0;
	  self->noiseLevel = MIN_NOISE;
	  self->envelope = MIN_NOISE;

	  return 0;
}


int SimpleVadFree(SimpleVadState* handle)
{
	if (NULL == handle)
	{
		return -1;
	}
	sdram_free(handle);

	return 0;
}


int VadZcr(SimpleVadState *st, short *x, int len )
{
	int i;
	if( (NULL == st) || (NULL == x) )
	{
		return -1;
	}

	st->zcr = 0;
	for( i = 1; i< len ; i++)
	{		
		st->zcr = st->zcr + abs( VAD_SIGN( x[i] ) - VAD_SIGN( x[i-1] ) );
	}
	st->zcr = st->zcr/2;

	return 0;
}

int SimpleVadProc(SimpleVadState *st,  short *pSrc,  int numOfSrcSamps)
{
	int i = 0;
	int flag = 0;
	float totLevel = 0.0f;


	if( (NULL == st) || (NULL == pSrc) )
	{
		return -1;
	}

	//1.compute the max level and avg level
	st->maxlevel = 0;

	for( i=0; i<numOfSrcSamps ;i++ )
	{
		short temp = abs(pSrc[i]);	
		totLevel = totLevel + temp;
		st->maxlevel = st->maxlevel > temp ? st->maxlevel : temp;
	}
	st->avglevel = totLevel / numOfSrcSamps;

	VadZcr(st, pSrc, numOfSrcSamps);

	if(0 == st->zcr)
	{
		st->ezcr = 0;
	}
	else
	{
		st->ezcr = st->avglevel / st->zcr ;
	}
	//__android_log_print(ANDROID_LOG_INFO, "SimpleVad", "SimpleVadProcess: avglevel=%f, zcr=%d, ezcr=%f", st->avglevel, st->zcr, st->ezcr);

	//2.compute the background noise
	if (st->envelope < st->maxlevel  )
	{
		st->envelope = st->envelope * 0.99f + st->maxlevel * 0.01f;
	}
	else
	{
		st->envelope = st->maxlevel;
	}

	if (st->maxlevel < st->envelope * 2.0f  )
	{
		st->noiseLevel = st->noiseLevel * 0.99f + st->maxlevel * 0.01f;
	}

	if( st->noiseLevel > MAX_NOISE )
	{
		st->noiseLevel = MAX_NOISE;
	}

	if( st->noiseLevel < MIN_NOISE )
	{
		st->noiseLevel = MIN_NOISE;
	}

	//__android_log_print(ANDROID_LOG_INFO, "SimpleVad", "SimpleVadProcess: noiseLevel=%f, maxlevel=%d, envelope=%f", st->noiseLevel, st->maxlevel, st->envelope);

	//3.make vad decision
	if (st->maxlevel > st->noiseLevel * 2.0)
	{
		flag = 1;

		if( st->zcr < ZCR_MIN( numOfSrcSamps ) || st->zcr > ZCR_MAX( numOfSrcSamps ) || st->ezcr < EZCR_TH( numOfSrcSamps ) )
		{
			flag = 0;
		}

	}
	else
	{
		flag = 0;
	}

	if ( flag )
	{  
		st->burst_count++;
		if (st->burst_count >= BRUST)
		{
			st->hang_count = HANG;
		}
		st->vadflag = 1;
	}
	else
	{

		st->burst_count = 0;
		st->vadflag = 0;
		if (st->hang_count > 0)
		{
			st->hang_count--;
			st->vadflag = 1;
		}
	}

	return st->vadflag ;
}

