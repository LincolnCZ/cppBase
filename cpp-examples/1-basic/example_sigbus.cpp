// C program to demonstrate Bus Error
#include <stdlib.h>

int main(int argc, char **argv) {

#if defined(__GNUC__)
# if defined(__i386__)
    /* Enable Alignment Checking on x86 */
    __asm__("pushf\norl $0x40000,(%esp)\npopf");
# elif defined(__x86_64__)
    /* Enable Alignment Checking on x86_64 */
    __asm__("pushf\norl $0x40000,(%rsp)\npopf");
# endif
#endif

    /* malloc() always provides aligned memory */
    char *cptr = malloc(sizeof(int) + 1);

    /* Increment the pointer by one, making it
       misaligned */
    int *iptr = (int *) ++cptr;

    /* Dereference it as an int pointer, causing
       an unaligned access */
    *iptr = 42;

    /* Following accesses will also result in
       sigbus error.
       short *sptr;
       int    i;

       sptr = (short *)&i;

       // For all odd value increments, it will
       // result in sigbus.
       sptr = (short *)(((char *)sptr) + 1);
       *sptr = 100;    */

    return 0;
}
