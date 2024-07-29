/*
This is a function called by modules when they need the aid of a submodule, for now it only allows one level of drop,
at a time at least.*/
ustd_t problem_ctl(ustd_t problem, ustd_t solution, ustd_t function){
	Thread * thread = get_thread_object(void);
	if (thread->type != thread_types::MODULE){ return NULLPTR;}

	File * module = thread->double_link;
	File * helper = module->pool[problem]->pool[solution];

	//i would like to find a way to make this a non-ugly function...
	set_thread(helper->runtime->workers->pool[0]);
	Thread * helper_thread = get_thread_object(void);
	helper_thread->prior = thread; 		//no need for a mutex because son modules are only accessible by the fathers

	helper_thread->instruction_pointer = helper_thread->father->code->functions[function] + helper_thread->father->userspace_code;
	run_thread(helper_thread);	//see ioctl for the return
}
