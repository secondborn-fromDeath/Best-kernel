/*
These are macros with a special meaning to be expanded by the compiler
*/
#define memcpy(dest,source,bytes){	\
	for (ustd_t i = 0; i < bytes; ++i){	\
		((uchar_t *)dest)[i] = ((uchar_t *)source)[i];	\
	}	\
}
#define memset(dest,val,bytes){	\
	for (ustd_t i = 0; i < bytes; ++i){	\
		((uchar_t *)dest)[i] = val;	\
	}	\
}
