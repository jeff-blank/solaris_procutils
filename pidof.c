#include <stdio.h>
#include <sys/types.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include "get_pinfo.h"

static char	copyright_info[] = "pidof.c  1998/05/17  Jeff Blank <jfb@mr-happy.com>";

void
usage()
{
	fprintf(stderr,"usage:\tpidof [ -cu ] <filename>\n");
	fprintf(stderr,"\twhere <filename> is a currently-running process's filename\n");
	fprintf(stderr,"\twithout the directory component\n");
}

int
main(ac,av)
int    ac;
char **av;
{
	DIR		 *dir;
	char		**avp,
			  format[5];
	int		  comma=0,
			  fd,
			  found=0,
			  n,
			  my_uid = -1;
	PROC_INFO_T	  pinfo;
	struct dirent	 *dent;
	extern char	 *optarg;
	extern int	  optind,
			  opterr,
			  optopt;

	if ( ac < 2 ) {
		usage();
		exit(1);
	}

	n=opterr=0;

	while ( (n=getopt(ac,av,"cu")) != EOF ) {
		switch(n) {
			case 'c':
				comma=1;
				break;
			case 'u':
				my_uid = getuid();
				break;
			case '?':
				fprintf(stderr,"pidof: unknown flag: -%c\n",
					optopt);
				usage();
				exit(1);
		}
	}

	if ( optind == ac ) {
		usage();
		exit(1);
	}

	if ( (chdir("/proc")) < 0 ) {
		fprintf(stderr,"pidof: cannot chdir to /proc: %s\nSeek help immediately!\n",
			strerror(errno));
		exit(1);
	}

	if ( (dir=opendir(".")) == NULL ) {
		perror("opendir: /proc");
		exit(1);
	}

	strcpy(format,"%d");

	while( (dent=readdir(dir)) != NULL ) {

		if ( !strcmp(dent->d_name,".") || !strcmp(dent->d_name,"..") )
			continue;

		if ( (get_pinfo(dent->d_name,&pinfo,1)) < 0 )
			continue;

		avp = &(av[optind]);
		while ( *avp != NULL ) {
			if ( !(strncmp(*avp,pinfo.pr_fname,PRFNSZ)) &&
				((my_uid < 0) || (my_uid == pinfo.pr_uid)) ) {
				printf(format,pinfo.pr_pid);
				if ( !found ) {
					sprintf(format,"%c%%d",comma?',':' ');
					found=1;
				}
			}
			++avp;
		}
	}

	closedir(dir);

	if ( found )
		printf("\n");
	else
		exit(1);

	exit(0);
}
