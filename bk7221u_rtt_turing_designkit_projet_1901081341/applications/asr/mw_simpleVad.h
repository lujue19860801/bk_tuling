/*
 * This header file includes the Simple VAD API calls. Specific function calls are given below.
 */

#ifndef MW_SIMPLEVAD_H_  // NOLINT
#define MW_SIMPLEVAD_H_

typedef struct SimpleVadState_T VadInst;

#ifdef __cplusplus
extern "C" {
#endif

// Creates an instance to the VAD structure.
//
// - handle [o] : Pointer to the VAD instance that should be created.
//
// returns      : 0 - (OK), -1 - (Error)
int SimpleVadCreate(VadInst** handle);

// Frees the dynamic memory of a specified VAD instance.
//
// - handle [i] : Pointer to VAD instance that should be freed.
//
// returns      : 0 - (OK), -1 - (NULL pointer in)
int SimpleVadFree(VadInst* handle);

// Calculates a VAD decision for the |audio_frame|. For valid sampling rates
// frame lengths,320
//
// - handle       [i/o] : VAD Instance. Needs to be initialized by
// - audio_frame  [i]   : Audio frame buffer.
// - frame_length [i]   : Length of audio frame buffer in number of samples.
//
// returns              : 1 - (Active Voice),
//                        0 - (Non-active Voice),
//                       -1 - (Error)
int SimpleVadProc(VadInst* handle, short* audio_frame,
                      int frame_length);

#ifdef __cplusplus
}
#endif

#endif  // MW_SIMPLEVAD_H_  // NOLINT
