#ifndef MW_ASRVAD_H_  // NOLINT
#define MW_ASRVAD_H_


#define SAR_VAD_SMP_RATE			(8000)
#define SAR_VAD_FRM_LEN				(SAR_VAD_SMP_RATE/100)

#define SAR_VAD_MAX_DLY_FRM			(300/10)




#ifdef __cplusplus
extern "C" {
#endif

// Creates an instance to the VAD structure.
//
// - handle [o] : Pointer to the VAD instance that should be created.
//
// returns      : 0 - (OK), -1 - (Error)
int AsrVadCreate(void **handle, int nSmpRate);

// Frees the dynamic memory of a specified VAD instance.
//
// - handle [i] : Pointer to VAD instance that should be freed.
int AsrVadFree(void *handle);

// Calculates a VAD decision for the |audio_frame|. For valid sampling rates
// frame lengths, see the description of WebRtcVad_ValidRatesAndFrameLengths().
//
// - handle       [i/o] : VAD Instance. Needs to be initialized by
//                        WebRtcVad_Init() before call.
// - fs           [i]   : Sampling frequency (Hz): 8000, 16000, or 32000
// - audio_frame  [i]   : Audio frame buffer.
// - frame_length [i]   : Length of audio frame buffer in number of samples.
//
// returns              : -1 - error,
//                        0 - ok,
int AsrVadProcess(void *handle, short *wInData, short *wOutData, 
					int nDataLen, int *VadState);


#ifdef __cplusplus
}
#endif

#endif  // MW_ASRVAD_H_  // NOLINT
