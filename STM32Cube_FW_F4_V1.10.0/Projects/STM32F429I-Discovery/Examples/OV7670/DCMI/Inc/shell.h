
#ifndef _SHELL_H_
#define _SHELL_H_

struct _command {
	const char *name;
	void *proc_func;
};


void shell_processing( char *cmd , int cmd_max_length );


#endif
