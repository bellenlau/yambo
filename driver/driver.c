/*
  Copyright (C) 2000-2008 A. Marini and the YAMBO team 
               http://www.yambo-code.org
  
  This file is distributed under the terms of the GNU 
  General Public License. You can redistribute it and/or 
  modify it under the terms of the GNU General Public 
  License as published by the Free Software Foundation; 
  either version 2, or (at your option) any later version.
 
  This program is distributed in the hope that it will 
  be useful, but WITHOUT ANY WARRANTY; without even the 
  implied warranty of MERCHANTABILITY or FITNESS FOR A 
  PARTICULAR PURPOSE.  See the GNU General Public License 
  for more details.
 
  You should have received a copy of the GNU General Public 
  License along with this program; if not, write to the Free 
  Software Foundation, Inc., 59 Temple Place - Suite 330,Boston, 
  MA 02111-1307, USA or visit http://www.gnu.org/copyleft/gpl.txt.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#if defined _MPI 
 #include <mpi.h>
#endif
typedef struct 
{
        char *ln;
        char *sn;
        char *d;
        int   ni;
        int   nr;
        int   nc;
        int   st;
} Ldes;
#if defined _FORTRAN_US
 #define F90_FUNC(name,NAME) name ## _
 #define F90_FUNC_(name,NAME) name ## _
#else
 #define F90_FUNC(name,NAME) name
 #define F90_FUNC_(name,NAME) name
#endif
#include "codever.h"
#if defined _yambo  || _RAS || _REELS || _ELPH || _SC || _BIGSYS || _TB
 #include "yambo_cpp.h"
#endif
#if defined _ypp  || _YPP_ELPH || _YPP_RAS
 #include "ypp_cpp.h"
#endif
#if defined _a2y
 #include "a2y.h"
#endif
#if defined _f2y
 #include "f2y.h"
#endif
#if defined _p2y
 #include "p2y.h"
#endif
#if defined _e2y
 #include "e2y.h"
#endif
static void usage(int verbose);
static void title(FILE *file_name,char *cmnt);
main(int argc, char *argv[])
{
 int io,i,c,j,k,nf,lni,lnr,lnc,ttd,
     iif=0,iid=1,iod=1,icd=1,nr=0,ijs=0,np=1,pid=0;
/* 
 By default MPI_init is on. It is swiched off during the options scanning
*/
 int mpi_init=0;
 int iv[4];
 double rv[4];
 char *cv[4];
 char *fmt=NULL,*inf=NULL,*od=NULL,*id=NULL,*js=NULL,*db=NULL,*com_dir=NULL;
 extern int optind, optopt;
 extern int guess_winsize();
 char rnstr1[500]={'\0'},rnstr2[500]={'\0'},edit_line[100]={'\0'};
 struct stat buf;

/* 
 Default input file, Job string, I/O directories
*/
 inf = (char *) malloc(strlen(tool)+4);
 strcpy(inf,tool);
 strcat(inf,".in");
 iif=strlen(inf);
 id  = (char *) malloc(2);strcpy(id,".");
 od  = (char *) malloc(2);strcpy(od,".");
 com_dir  = (char *) malloc(2);strcpy(com_dir,".");
 js  = (char *) malloc(2);strcpy(js," ");

 ttd=guess_winsize();

 strcpy(rnstr2," ");
 if (argc>1) {
   while(opts[nr].ln!=NULL) {nr++;};
   fmt = (char *) malloc(sizeof(char)*nr+1);
 /* 
  strcat needs fmt to be initialized 
 */
   fmt[0] = '\0' ;
   for(i=0;i<=nr-1;i++) {
     strcat(fmt,opts[i].sn);
   }
   while ((c = getopt(argc, argv, fmt)) != -1) {
     io=optind;
     if (io==1) {io++;};
     for(i=0;i<=nr-1;i++) {
       if (strstr(argv[io-1],opts[i].sn)!=0 && opts[i].st==0) { j=i;break;};
     };
     if (c=='?') {usage(1);exit(0);};
 /*
   Upper Case actions
 */
     if (strcmp(opts[j].ln,"help")==0) {usage(1);exit(0);};
     if (strcmp(opts[j].ln,"lhelp")==0) {usage(2);exit(0);};
/* 
 Switch off MPI_init as I have options used to create the input file...
*/
#if defined _yambo  || _RAS || _REELS || _ELPH || _SC || _BIGSYS || _TB
     if (j> 10) {mpi_init=-1;};
#endif
#if defined _ypp || _YPP_ELPH || _YPP_RAS
     if (j> 6) {mpi_init=-1;};
#endif
/* 
 ... or if nompi is given 
*/
     if (strcmp(opts[j].ln,"nompi")==0) {mpi_init=-1;};

     opts[j].st++; lni=0;lnr=0;lnc=0;
     nf=opts[j].ni+opts[j].nr+opts[j].nc;
     if (optind+nf>argc) {
       fprintf(stderr,"\n%s : invalid option -- %s\n",tool,opts[j].sn); usage(1);exit(0);
     };
     for(i=1;i<=nf;i++) {
       k=0;
       if (strspn(argv[optind-1+i],"-")==1) {
         fprintf(stderr,"\n%s : invalid option -- %s\n",tool,opts[j].sn); usage(1);exit(0);
       };
       if (opts[j].ni!=0 && k==0) {lni++;iv[lni]=atoi(argv[optind-1+i]);opts[j].ni--;k=1;};
       if (opts[j].nr!=0 && k==0) {lnr++;rv[lnr]=atof(argv[optind-1+i]);opts[j].nr--;k=1;};
       if (opts[j].nc!=0 && k==0) {lnc++;cv[lnc]=argv[optind-1+i];opts[j].nc--;k=1; };
     };
 /* 
   Input File, i/o directory 
 
   REALLOC ! 
 */
     if (strcmp(opts[j].ln,"ifile")==0) {
       free(inf);
       inf = (char *) malloc(strlen(cv[1])+1);  
       strcpy(inf,cv[1]);
       iif=strlen(inf);
     };
     if (strcmp(opts[j].ln,"idir")==0) {
       free(id);
       id = (char *) malloc(strlen(cv[1]));
       strcpy(id,cv[1]);
       iid=strlen(id);
     };
     if (strcmp(opts[j].ln,"odir")==0) {
       free(od);
       od = (char *) malloc(strlen(cv[1]));
       strcpy(od,cv[1]);
       iod=strlen(od);
     };
     if (strcmp(opts[j].ln,"cdir")==0) {
       free(com_dir);
       com_dir = (char *) malloc(strlen(cv[1]));
       strcpy(com_dir,cv[1]);
       icd=strlen(com_dir);
     };
     if (strcmp(opts[j].ln,"jobstr")==0) {
       free(js);
       js = (char *) malloc(strlen(cv[1]));
       strcpy(js,cv[1]);
       ijs=strlen(js);
     };
     /* ------------------------- */
     strcat(rnstr1," ");
     strcat(rnstr1,opts[j].ln);
     strcpy(rnstr2,rnstr1);
     for(i=1;i<=lni;i++) {sprintf(rnstr1,"%s %d ",rnstr2,iv[i]);strcpy(rnstr2,rnstr1);};
     for(i=1;i<=lnr;i++) {sprintf(rnstr1,"%s %f ",rnstr2,rv[i]);strcpy(rnstr2,rnstr1);};
     for(i=1;i<=lnc;i++) {sprintf(rnstr1,"%s %s ",rnstr2,cv[i]);strcpy(rnstr2,rnstr1);};

   };
 };
/* 
 If id/od/com_dir are not found switch to the deafult i/o directory 
*/
 lni=strlen(rnstr2);
 if (stat(id, &buf) != 0) {strcpy(id,".");iid=1;};
 if (stat(od, &buf) != 0) {strcpy(od,".");iod=1;};
 if (stat(com_dir, &buf) != 0) {strcpy(com_dir,".");icd=1;};
 /* 
   MPI
 ===========================================================================
 */
#if defined _MPI
 if (mpi_init==0) {
   MPI_Init(&argc,&argv);               /* starts MPI */
   MPI_Comm_rank(MPI_COMM_WORLD, &pid); /* get current process id */
   MPI_Comm_size(MPI_COMM_WORLD, &np);  /* get number of processes */
 };
#endif
#if defined _yambo  || _RAS || _REELS || _ELPH || _SC || _BIGSYS || _TB
 /* 
   Running the Fortran YAMBO driver 
 ===========================================================================
 */
 F90_FUNC(yambo_driver,YAMBO_DRIVER)(
          rnstr2,&lni,inf,&iif,id,&iid,od,&iod,com_dir,&icd,js,&ijs,&np,&pid);
#endif
#if defined _ypp || _YPP_ELPH || _YPP_RAS
 /* 
   Running the Fortran YPP driver
 ===========================================================================
 */
 F90_FUNC(ypp_i,YPP_I)(
          rnstr2,&lni,inf,&iif,id,&iid,od,&iod,com_dir,&icd,js,&ijs,&np,&pid);
#endif
#if defined _a2y 
 /* 
   Running the Fortran a2y driver
 ===========================================================================
 */
 F90_FUNC(a2y_i,A2Y_I)(
          rnstr2,&lni,inf,&iif,id,&iid,od,&iod,com_dir,&icd,js,&ijs,&np,&pid);
#endif
#if defined _p2y
 /* 
   Running the Fortran p2y driver 
 ===========================================================================
 */
 F90_FUNC(p2y_i,P2Y_I)(
          rnstr2,&lni,inf,&iif,id,&iid,od,&iod,com_dir,&icd,js,&ijs,&np,&pid);
#endif
#if defined _e2y 
 /* 
   Running the Fortran p2y driver 
 ===========================================================================
 */
 F90_FUNC(e2y_i,E2Y_I)(
          rnstr2,&lni,inf,&iif,id,&iid,od,&iod,com_dir,&icd,js,&ijs,&np,&pid);
#endif
#if defined _f2y 
 /* 
   Running the Fortran f2y driver 
 ===========================================================================
 */
 F90_FUNC(f2y_i,F2Y_I)(
          rnstr2,&lni,inf,&iif,id,&iid,od,&iod,com_dir,&icd,js,&ijs,&np,&pid);
#endif
 /* 
   INPUT FILE
 ===========================================================================
 */
 strcpy(edit_line,"vim ");
 strncat(edit_line,inf,strlen(inf));
#if defined _yambo || _RAS || _REELS || _ypp || _ELPH || _SC || _YPP_ELPH || _YPP_RAS  || _BIGSYS|| _TB
 if (iif == 1 && ttd>0 ) 
 {
  system(edit_line);
  exit (0);
 };
#endif
 /* 
   Error message
 ===========================================================================
 */
 if ( iif < 0 ) 
 {
  if (pid==0 && iif == -1) {
   fprintf(stderr," \n%s\n\n","yambo: cannot access CORE database (SAVE/*db1 and/or SAVE/*wf)");
  };
  if (pid==0 && iif == -2) {
   fprintf(stderr," \n%s\n\n","yambo: invalid command line options and/or build");
  };
#if defined _MPI
  if (np>1) {
   MPI_Barrier(MPI_COMM_WORLD);
   MPI_Finalize();
  };
#endif 
 };
 /* 
   CLEAN & EXIT
 ===========================================================================
 */
 free(inf);
 free(fmt);
 free(id);
 free(js);
 free(od); 
 free(db);
 exit(0);
}
static void usage(int verbose)
{
 int i,j,nr=0;
 while(opts[nr].ln!=NULL) {nr++;};
 if (verbose==1) {
  fprintf(stderr,"\nThis is %s %s\n",tool,codever); 
  fprintf(stderr,"Usage: %s",tool); 
  for(j=0;j<=nr-1;j++)
   {fprintf(stderr," -%s",opts[j].sn);
   for(i=1;i<=opts[j].ni;i++) {fprintf(stderr," %s","<int>");};
   for(i=1;i<=opts[j].nr;i++) {fprintf(stderr," %s","<real>");};
   for(i=1;i<=opts[j].nc;i++) {fprintf(stderr," %s","<opt>");};
  };
  fprintf(stderr,"\n%s%s%s\n","Try `",tool," -H' for more information");exit(0);
 };
 if (verbose==2) {title(stderr,"");
 for(j=0;j<=nr-1;j++)
  {fprintf(stderr," -%s",opts[j].sn);
   for(i=1;i<=opts[j].ni;i++) {fprintf(stderr," %s","<int>");};
   for(i=1;i<=opts[j].nr;i++) {fprintf(stderr," %s","<real>");};
   for(i=1;i<=opts[j].nc;i++) {fprintf(stderr," %s","<opt>");};
   if (opts[j].ni==0 && opts[j].nr==0 && opts[j].nc==0) {fprintf(stderr,"\t");};
   fprintf(stderr,"\t:%s\n",opts[j].d);
  };
  fprintf(stderr,"\n");
  fprintf(stderr,"%s\t%s\n\t%s\n\n","By","YAMBO developers group",
                 "http://www.yambo-code.org");
 };
};
static void title(FILE *file_name,char *cmnt)
{
 fprintf(file_name,"%s%s\n",cmnt,  " ___ __  _____  __ __  _____   _____ ");
 fprintf(file_name,"%s%s\n",cmnt,  "|   Y  ||  _  ||  Y  ||  _  \\ |  _  |");
 fprintf(file_name,"%s%s\n",cmnt,  "|   |  ||. |  ||.    ||. |  / |. |  |");
 fprintf(file_name,"%s%s\n",cmnt,  " \\   _/ |. _  ||.\\ / ||. _  \\ |. |  |");
 fprintf(file_name,"%s%s\n",cmnt,  "  |: |  |: |  ||: |  ||: |   \\|: |  |");
 fprintf(file_name,"%s%s\n",cmnt,  "  |::|  |:.|:.||:.|:.||::.   /|::.  |");
 fprintf(file_name,"%s%s\n",cmnt,  "  `--\"  `-- --\"`-- --\"`-----\" `-----\"");
 fprintf(file_name,"%s\n%s Tool: %s %s\n",cmnt,cmnt,tool,codever);
 fprintf(file_name,"%s Description: %s \n\n",cmnt,tdesc);
};
