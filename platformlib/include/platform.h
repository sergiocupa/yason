#ifndef PLATFORM_LIB_H
#define PLATFORM_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

	#ifdef _WIN32 || _WIN64
	    #define PLATFORM_WIN
	#endif 


	#ifdef PLATFORM_WIN
	    #define PLATFORM_API __declspec ( dllexport )
	#else 
	    #define PLATFORM_API
	#endif 


#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_LIB */