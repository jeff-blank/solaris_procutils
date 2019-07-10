#include <stdio.h>
#include <sys/types.h>

#include <fcntl.h>

#include "get_pinfo.h"

int
get_pinfo(char *d_fname, PROC_INFO_T *info, int quiet)
{
	int	fd,
		n;
	char	p_fname[15];

#ifdef SOL26
	sprintf(p_fname,"%s/psinfo",d_fname);
#else
	strcpy(p_fname,d_fname);
#endif
	if ( (fd=open(p_fname,O_RDONLY,0)) < 0 ) {
		if ( !quiet )
			perror(p_fname);
		return -1;
	}

#ifdef SOL26
	if ( (n=read(fd,info,sizeof(*info))) < 0 ) {
		if ( !quiet )
			perror(p_fname);
		close(fd);
		return -1;
	}
	else if ( !n ) {
		if ( !quiet )
			fprintf(stderr,"0 bytes read from /proc/%s!\n",p_fname);
		close(fd);
		return -1;
	}
#else
	if ( (ioctl(fd,PIOCPSINFO,info)) < 0 ) {
		if ( !quiet )
			perror("ioctl: PIOCPSINFO");
		close(fd);
		return -1;
	}
#endif

	close(fd);
	return 0;
}
