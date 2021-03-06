/* See LICENCE file for copyright and licence details */

#define MIN(A, B) (((A) > (B)) ? (B) : (A))
#define MAX(A, B) (((A) < (B)) ? (B) : (A))

void	 error(const char *, ...);
void	 warning(const char *, ...);
void	*emalloc(unsigned int);
void	*erealloc(void *, unsigned int);
char	*estrdup(const char *);
long	 estrtol(const char *, int base);
