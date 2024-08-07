template <typename T>
T tosmallpage(T x){
	if (x%4096){ x = x/4096+1;}
	else { x /= 4096;}
	return x;
}
template <typename T>
T pagetype_roundown(ustd_t pagetype, T val){
	if !(val){ return val;}
	return val/get_multi_from_pagetype(pagetype);
}
template <typename T>
T pagetype_roundup(T integer, ustd_t pagetype){
	ustd_t divider = get_multi_from_pagetype(pagetype);
	if (integer%divider){ integer /= divider;}
	else{ integer /= divider; ++integer;}
	return integer;
}
