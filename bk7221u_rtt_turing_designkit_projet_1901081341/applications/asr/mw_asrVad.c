#include <stdlib.h>

#include "mw_simpleVad.h"
#include "mw_asrVad.h"
	

typedef  struct  
{
	short data[2*SAR_VAD_FRM_LEN];
	int	  vadState;
}vadDataStruc;

typedef  struct asrVadInst_T
{
	vadDataStruc   vadData[SAR_VAD_MAX_DLY_FRM];
	int m_nPopPtr;
	int m_nPushPtr;
	int m_nContinueSilenceCnt;
	int m_bTrueVadState;
	int m_nSmpRate;
	//NsxHandle  *nsxInst;
	VadInst *vadInst;
}asrVadInst;


int AsrVadCreate(void **handle, int nSmpRate)
{
	asrVadInst *self = NULL;
	int i,rtn = 0;

	if (handle == NULL) 
	{
		return -1;
	}
	*handle = (asrVadInst *) sdram_malloc(sizeof(asrVadInst));
	self = *handle;
	if (NULL == *handle) 
	{
		return -1;
	}

	//init asrVadInst
	for (i=0; i<SAR_VAD_MAX_DLY_FRM; i++)
	{
		memset(self->vadData[i].data, 0, 2*2*SAR_VAD_FRM_LEN);
		self->vadData[i].vadState = 0;
	}
	self->m_nPushPtr = 0;
	self->m_nPopPtr = 1;
	self->m_nContinueSilenceCnt = 0;
	self->m_bTrueVadState = 0;
	self->m_nSmpRate = nSmpRate;
	//self->nsxInst = NULL;
	self->vadInst = NULL;

	//init ns & vad
	//rtn = WebRtcNsx_Create(&(self->nsxInst));
	rtn |= SimpleVadCreate(&(self->vadInst));

	//rtn |= WebRtcNsx_Init(self->nsxInst, SAR_VAD_SMP_RATE);
	//rtn |= WebRtcVad_Init(self->vadInst);	

	//rtn |= WebRtcNsx_set_policy(self->nsxInst, 3);
	//rtn |= WebRtcVad_set_mode(self->vadInst, 3);

	if (rtn)
	{
// 		if (self->nsxInst)
// 		{
// 			WebRtcNsx_Free(self->nsxInst);
// 		}
		if (self->vadInst)
		{
			SimpleVadFree(self->vadInst);
		}			
		return -1;
	}
	return 0;	
}

int AsrVadProcess(void *vadInst, short *wInData, short *wOutData, int nDataLen, int *VadState)
{
	short tmpBuf0[2*SAR_VAD_FRM_LEN];
	const short* const* inPtr[2];
	short* const* outPtr[2];
	asrVadInst *self = (asrVadInst *)vadInst;
	int i, vad=0, vadSum=0,threshold;

	if ((NULL == self) || (NULL == wInData) || (NULL == wOutData) || (NULL == VadState))
	{
		return -1;
	}
	if (nDataLen != self->m_nSmpRate / 100)
	{
		return -1;
	}

	//downsample input data if needed
	if (2*SAR_VAD_SMP_RATE == self->m_nSmpRate)
	{
		for (i=0; i<SAR_VAD_FRM_LEN; i++)
		{
			tmpBuf0[i] = wInData[2*i];
		}
		inPtr[0] = (const short *const *)tmpBuf0;
		outPtr[0] = (short *const *)tmpBuf0;
	}
	else
	{
		inPtr[0] = (const short *const *)wInData;
		outPtr[0] = (short *const *)wOutData;
	}	

	//preprocess, check current frm's vad	
	//mw_Noise_Process(self->nsxInst, inPtr, 1, outPtr);
	vad = SimpleVadProc(self->vadInst, (short*)inPtr[0], SAR_VAD_FRM_LEN);

// 	*VadState = vad;
// 	memcpy((char *)wOutData,(char *)wInData, 2*nDataLen);
// 	return 0;

	//pop data
	memcpy((char *)tmpBuf0, self->vadData[self->m_nPopPtr].data, 2*nDataLen);
	self->m_nPopPtr ++;
	self->m_nPopPtr = self->m_nPopPtr % SAR_VAD_MAX_DLY_FRM;

	//push data
	if (SAR_VAD_SMP_RATE == self->m_nSmpRate)
	{
		memcpy((char *)self->vadData[self->m_nPushPtr].data,(char *)inPtr[0], nDataLen*2);
	} 
	else //16khz,output without ns 
	{
		memcpy((char *)self->vadData[self->m_nPushPtr].data,(char *)wInData, nDataLen*2);
	}	
	self->vadData[self->m_nPushPtr].vadState = vad;
	self->m_nPushPtr ++;
	self->m_nPushPtr = self->m_nPushPtr % SAR_VAD_MAX_DLY_FRM;

	//check real silence
	if (0 == vad)
	{
		self->m_nContinueSilenceCnt++;
		threshold = 15;
		if (self->m_nContinueSilenceCnt > threshold)  //100ms
		{
			self->m_bTrueVadState = 0;
		}
	}
	else
	{
		self->m_nContinueSilenceCnt = 0;
	}

	//check real voice active
	vadSum = 0;
	for (i=0; i<SAR_VAD_MAX_DLY_FRM; i++)
	{
		if (self->vadData[i].vadState)
		{
			vadSum++;
		}
	}
	if (vadSum >= SAR_VAD_MAX_DLY_FRM - 15)
	{
		self->m_bTrueVadState = 1;
	}

	*VadState = self->m_bTrueVadState;
	memcpy((char *)wOutData,(char *)tmpBuf0, 2*nDataLen);

	return 0;

}

int AsrVadFree(void *vadInst)
{
	asrVadInst *self = (asrVadInst *)vadInst;
	if (NULL == self)
	{
		return -1;
	}

// 	if (self->nsxInst)
// 	{
// 		WebRtcNsx_Free(self->nsxInst);
// 		self->nsxInst = NULL;
// 	}

	if (self->vadInst)
	{
		SimpleVadFree(self->vadInst);
		self->vadInst = NULL;
	}	

	sdram_free(self);
	self = NULL;
	return 0;
}
