/* welcome.c -- sanity check for the local COS 217 environment.
   Modeled on the program from lecture 1. */

#include <stdio.h>

/* Print a greeting; return 0 to indicate success. */
int main(int argc, char *argv[])
{
   int iArgument;

   for (iArgument = 0; iArgument < argc; iArgument++)
      printf("Argument %d: %s\n", iArgument, argv[iArgument]);

   printf("Welcome to COS 217\n");
   printf("Introduction to Programming Systems\n\n");
   printf("%s %d\n", "Fall 2026 (self-study)", 2026);
   return 0;
}
