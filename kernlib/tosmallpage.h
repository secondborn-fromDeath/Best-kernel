#define tosmallpage(x){	\
	if (x%4096){ x = x/4096+1;}	\
	else { x /= 4096;}	\
}
