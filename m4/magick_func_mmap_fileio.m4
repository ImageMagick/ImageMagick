# MAGICK_FUNC_MMAP_FILEIO
# ------------
AC_DEFUN([MAGICK_FUNC_MMAP_FILEIO],
[AC_CHECK_HEADERS(stdlib.h unistd.h)
AC_CHECK_FUNCS(getpagesize)
AC_CACHE_CHECK(for working mmap file i/o, magick_cv_func_mmap_fileio,
[AC_RUN_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT]
[[/* malloc might have been renamed as rpl_malloc. */
#undef malloc

/*
   This test is derived from GNU Autoconf's similar macro.
   The purpose of this test is to verify that files may be memory
   mapped, and that memory mapping and file I/O are coherent.

   The test creates a test file, memory maps the file, updates
   the file using the memory map, and then reads the file using
   file I/O to verify that the file contains the updates.
*/

#include <fcntl.h>
#include <sys/mman.h>

#if !STDC_HEADERS && !HAVE_STDLIB_H
char *malloc ();
#endif

/* This mess was copied from the GNU getpagesize.h.  */
#if !HAVE_GETPAGESIZE
/* Assume that all systems that can run configure have sys/param.h.  */
# if !HAVE_SYS_PARAM_H
#  define HAVE_SYS_PARAM_H 1
# endif

# ifdef _SC_PAGESIZE
#  define getpagesize() sysconf(_SC_PAGESIZE)
# else /* no _SC_PAGESIZE */
#  if HAVE_SYS_PARAM_H
#   include <sys/param.h>
#   ifdef EXEC_PAGESIZE
#    define getpagesize() EXEC_PAGESIZE
#   else /* no EXEC_PAGESIZE */
#    ifdef NBPG
#     define getpagesize() NBPG * CLSIZE
#     ifndef CLSIZE
#      define CLSIZE 1
#     endif /* no CLSIZE */
#    else /* no NBPG */
#     ifdef NBPC
#      define getpagesize() NBPC
#     else /* no NBPC */
#      ifdef PAGESIZE
#       define getpagesize() PAGESIZE
#      endif /* PAGESIZE */
#     endif /* no NBPC */
#    endif /* no NBPG */
#   endif /* no EXEC_PAGESIZE */
#  else /* no HAVE_SYS_PARAM_H */
#   define getpagesize() 8192	/* punt totally */
#  endif /* no HAVE_SYS_PARAM_H */
# endif /* no _SC_PAGESIZE */

#endif /* no HAVE_GETPAGESIZE */

int
main ()
{
  char *data, *data2, *data3;
  int i, pagesize;
  int fd;

  pagesize = getpagesize ();

  /* First, make a file with some known garbage in it. */
  data = (char *) malloc (pagesize);
  if (!data)
    exit (1);
  for (i = 0; i < pagesize; ++i)
    *(data + i) = rand ();
  umask (0);
  fd = creat ("conftest.mmap", 0600);
  if (fd < 0)
    exit (1);
  if (write (fd, data, pagesize) != pagesize)
    exit (1);
  close (fd);

  /* Mmap the file as read/write/shared and verify that we see the
  same garbage. */
  fd = open ("conftest.mmap", O_RDWR);
  if (fd < 0)
    exit (1);
  data2 = mmap (0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0L);
  if (data2 == 0)
    exit (1);
  for (i = 0; i < pagesize; ++i)
    if (*(data + i) != *(data2 + i))
      exit (1);

  /* Finally, make sure that changes to the mapped area 
     percolate back to the file as seen by read().  */
  for (i = 0; i < pagesize; ++i)
    *(data2 + i) = *(data2 + i) + 1;
  data3 = (char *) malloc (pagesize);
  if (!data3)
    exit (1);
  if (read (fd, data3, pagesize) != pagesize)
    exit (1);
  for (i = 0; i < pagesize; ++i)
    if (*(data2 + i) != *(data3 + i))
      exit (1);
  close (fd);
  exit (0);
}]])],
	       [magick_cv_func_mmap_fileio=yes],
	       [magick_cv_func_mmap_fileio=no],
	       [magick_cv_func_mmap_fileio=no])])
if test $magick_cv_func_mmap_fileio = yes; then
  AC_DEFINE(HAVE_MMAP_FILEIO, 1,
	    [Define to 1 if you have a working `mmap' system call.])
fi
rm -f conftest.mmap
])# MAGICK_FUNC_MMAP_FILEIO
