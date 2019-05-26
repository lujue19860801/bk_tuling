#ifndef _ASR_CONFIG_H_
#define _ASR_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif

	/*	 asr service configuration	 */
#define    	ASR_SERVICE_UNKNOWN					(0)
#define		ASR_SERVICE_ROOBO					(1)
#define    	ASR_SERVICE_TULING					(2)
#define		ASR_SERVICE_BAIDU					(3)
	
	/*	 wake word engine configuration   */
#define	WAKEWORD_ENGINE_UNKNOWN			(0)
#define	WAKEWORD_ENGINE_IFLY				(1)
#define    WAKEWORD_ENGINE_ROOBO_FLOAT		(2)
#define 	WAKEWORD_ENGINE_ROOBO_FIXPOINT		(3)
#define	WAKEWORD_ENGINE_ROOBO_FIXPOINT_VAD (4)
	
#define	PBOOK_TYPE_NONE						(0)
#define 	PBOOK_TYPE_MW						(1)
#define 	PBOOK_TYPE_TULING					(2)



#define	ASR_SERVICE_TYPE						(ASR_SERVICE_TULING)


#ifdef __cplusplus
}
#endif
#endif
