struct opttype {
	char *longname;
	char shortname;
	char type;
	void *outdata;
};

#define OPTTYPE_BOOL	1
#define OPTTYPE_CHAR	2
#define OPTTYPE_SHORT	3
#define OPTTYPE_USHORT	4
#define OPTTYPE_INT	5
#define OPTTYPE_UINT	6
#define OPTTYPE_LONG	7
#define OPTTYPE_ULONG	8
#define OPTTYPE_LONGLONG 9
#define OPTTYPE_ULONGLONG 10
#define OPTTYPE_FLOAT	11
#define OPTTYPE_DOUBLE	12
#define OPTTYPE_LONGDOUBLE 13
#define OPTTYPE_STRING	14

void fetchopts(int *argc, char ***argv, struct opttype *opts);
