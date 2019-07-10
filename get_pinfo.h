#ifndef _GET_PINFO_H
#define _GET_PINFO_H

#ifdef SOL26

# include <procfs.h>
# define PROC_INFO_T psinfo_t

#else
# include <sys/procfs.h>
# define PROC_INFO_T prpsinfo_t
#endif

#endif
