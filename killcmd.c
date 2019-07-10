#include <stdio.h>
#include <sys/types.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>

#include "get_pinfo.h"

char	**AV;
int	  AC;

static char	copyright_info[] = "killcmd.c  12/3/1998  Jeff Blank <jfb@mr-happy.com>";

typedef struct intlist {
	int		 num;
	struct intlist	*next;
} intlist;

char *sigmap[] = {
	"NULL",
	"HUP",		"INT",		"QUIT",		"ILL",
	"TRAP",		"ABRT",		"EMT",		"FPE",
	"KILL",		"BUS",		"SEGV",		"SYS",
	"PIPE",		"ALRM",		"TERM",		"USR1",
	"USR2",		"CHLD",		"PWR",		"WINCH",
	"URG",		"POLL",		"STOP",		"TSTP",
	"CONT",		"TTIN",		"TTOU",		"VTALRM",
	"PROF",		"XCPU",		"XFSZ",		"WAITING",
	"LWP",		"FREEZE",	"THAW",		"CANCEL",
	(char*)0
};

int
getsignum(char *signame)
{
	char	 *cp,
		**cpp;
	int	  i=0,
		  number=1;

	++signame;	/* move past leading '-'                   */
			/* this function is not entered without it */

	cp = signame;
	while( *cp != '\0' ) {
		if ( (*cp < '0') || (*cp > '9') ) {
			number = 0;
			break;
		}
		++cp;
	}

	/* ugly SIGABRT -> SIGIOT hack */
		if ( !(strncmp(signame,"IOT",3)) && (signame[3] == '\0') ) {
		return SIGABRT;
	}

	if ( number )
		return atoi(signame);

	cpp = sigmap;
	while ( *cpp != NULL ) {
		if ( !strncmp(signame,*cpp,strlen(*cpp)) )
			return i;
		++i;
		++cpp;
	}

	fprintf(stderr,"killcmd: unknown signal: SIG%s\nuse 'killcmd -l' to list signals.\n",
		signame);

	return -1;
}

int
is_signame(char *username, int oi)
{
	char	**sp,
		  tname[10];
	int	  i;

	memset(tname,'\0',sizeof(tname));
	*tname = 'U';
	strcat(tname,username);
	sp = sigmap;
	while ( *sp != NULL ) {
		if ( !(strcmp(tname,*sp)) && !(strncmp(AV[oi-1],"-U",2)) )
			return 1;
		++sp;
	}
	return 0;
}

void
usage(FILE *stream)
{
	fprintf(stream,"usage:\tkillcmd [ -qu ] [ -U user ] [ -SIG ] <filename>\n");
	fprintf(stream,"\t  where <filename> is a currently-running process's filename\n");
	fprintf(stream,"\t  without the directory component\n");
	fprintf(stream,"\tkillcmd -l\tto list signals\n");
	fprintf(stream,"\tkillcmd -h\tto display this help\n");
}

int
main(int ac, char **av)
{
	DIR		 *dir;
	char		**avp,
			 *cp,
			  killuser[9];
	int		  fd,
			  found=0,
			  c,
			  done,
			  killuid=-1,
			  hflag,
			  numsig=0,
			  lflag,
			  quiet=0,
			  me_only=0,
			  user_only=0,
			  my_uid=getuid(),
			  use_sig=SIGTERM;
	PROC_INFO_T	  pinfo;
	struct dirent	 *dent;
	struct intlist	 *newpid,
			 *pidp,
			 *pidlist;
	struct passwd	 *killuserE;
	extern char	 *optarg;
	extern int	  optind,
			  opterr,
			  optopt;

	AV = av;
	AC = ac;
	c=done=quiet=hflag=lflag=me_only=opterr=0;
	memset(killuser,'\0',sizeof(killuser));

	while ( (c=getopt(ac,av,"hlquU:")) != EOF ) {
		switch(c) {
			case 'h':
				hflag=1;
				break;
			case 'l':
				lflag=1;
				break;
			case 'q':
				quiet=1;
				break;
			case 'u':
				if ( killuid >= 0 ) {
					/* -U has already been specified */
					fprintf(stderr,"killcmd: use of -U excludes use of -u\n");
					exit(1);
				}
				me_only=1;
				killuid = my_uid;
				break;
			case 'U':
				strncpy(killuser,optarg,sizeof(killuser)-1);
				if ( is_signame(killuser,optind) ) {
					--optind;
					done = 1;
					break;
				}
				if ( (killuserE=getpwnam(killuser)) == NULL ) {
					fprintf(stderr,"killcmd: %s: unknown user\n",
						killuser);
					exit(1);
				}
				if ( me_only ) {
					/* -u has already been specified */
					fprintf(stderr,"killcmd: use of -u excludes use of -U\n");
					exit(1);
				}
				if ( killuid >= 0 ) {
					fprintf(stderr,"killcmd: %s: multiple-user kills not implemented\n",
						killuser);
				}
				killuid = killuserE->pw_uid;
				user_only=1;
				break;
			case '?':
				if ( (optopt >= 'A')&&(optopt <= 'Z') ) {
					done = 1;
				}
				else if ( (optopt >= '0')&&(optopt <= '9') ) {
					done = 1;
					numsig = 1;
				}
				else {
					fprintf(stderr,"killcmd: unknown flag: -%c\n",optopt);
					usage(stderr);
					exit(1);
				}
		}
		if(done)break;
	}

	if ( ((lflag||hflag) && (quiet||me_only||done||user_only)) ||
		(lflag && hflag) )
	{
		usage(stderr);
		exit(1);
	}
	else if ( lflag ) {
		printf("%s",sigmap[0]);
		avp = sigmap+1;
		while ( *avp != NULL )
			printf(" %s",*(avp++));
		printf("\n");
		exit(0);
	}
	else if ( hflag ) {
		usage(stdout);
		exit(0);
	}
	if ( optind >= ac ) {
		usage(stderr);
		exit(1);
	}

	if ( numsig && (*av[optind] != '-' ) )
		--optind;
	if ( *av[optind] == '-' ) {
		if ( (use_sig = getsignum(av[optind])) < 0 )
			exit(1);
		else
			++optind;
	}

	if ( optind >= ac ) {
		usage(stderr);
		exit(1);
	}

	if ( (chdir("/proc")) < 0 ) {
		fprintf(stderr,"Cannot chdir to /proc: %s\nSeek help immediately!\n",
			strerror(errno));
		exit(1);
	}

	if ( (dir=opendir(".")) == NULL ) {
		perror("opendir: /proc");
		exit(1);
	}

	pidlist = NULL;

	while( (dent=readdir(dir)) != NULL ) {

		if ( !strcmp(dent->d_name,".") || !strcmp(dent->d_name,"..") )
			continue;

		if ( (get_pinfo(dent->d_name,&pinfo,quiet)) < 0 )
			continue;

		avp = &(av[optind]);
		while ( *avp != NULL ) {
			if ( (!(strncmp(*avp,pinfo.pr_fname,PRFNSZ))) &&
				(
				((killuid >= -1) && (pinfo.pr_uid == killuid))
				|| (!me_only && !user_only)
				)
			   )
			{
				newpid=(intlist*)malloc(sizeof(intlist));
				newpid->num = pinfo.pr_pid;
				newpid->next = pidlist;
				pidlist = newpid;
				if ( !found )
					found=1;
			}
			++avp;
		}
	}

	closedir(dir);

#ifndef SOL26
	seteuid(my_uid);
#endif

	if ( !found ) {
		if ( !quiet )
			fprintf(stderr,"killcmd: no processes found\n");
		exit(1);
	}

	pidp = pidlist;
	while ( pidp != NULL ) {
		if ( ((kill(pidp->num,use_sig)) < 0) && !quiet ) {
			fprintf(stderr,"killcmd: %d: %s\n",pidp->num,
				strerror(errno));
		}
		pidp = pidp->next;
	}

	exit(0);
}
