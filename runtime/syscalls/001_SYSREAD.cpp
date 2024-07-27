volatile void SYSREAD(ustd_t descriptor, void * buf, ulong_t amount, ulong_t offset){
	Virtual_fs * vfs = get_vfs_object(void);
	void * truebuf = get_kingmem_object(void)->vmto_phys(buf);

	vfs->read(get_process_object(void)->descs[descriptor]->findex,truebuf,amount,offset);
}
