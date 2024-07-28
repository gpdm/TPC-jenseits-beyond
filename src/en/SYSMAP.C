#include <stdio.h>
#include <bios.h>
#include <dos.h>

#define length 30
#define maxblocks 32
#define CGA 1
#define MDA 2
#define Hercules 3
#define EGA 4

/* --------------------------------------------------- */
/* Check if RAM exists in given memory block           */
/* --------------------------------------------------- */

char ramexist (char blockno)
{   unsigned int far *ram;
    unsigned int j,test;

    ram=MK_FP(blockno*0x800,0);
    for (j=0; j<=0x3FFF; j++)
       { test=*ram=~*ram;
       if (test==*ram)
          { *ram=~*ram;
            return (1); }
       else
	  ram++; }
    return(0);
    }

/* ------------------ */
/* Set Video Mode     */
/* ------------------ */

void setmode(char mode)
{   union REGS regs;

    regs.h.ah = 0;
    regs.h.al = mode;
    int86(0x10,&regs,&regs);
}

/* ------------------------------- */
/* Get current Video Mode          */
/* ------------------------------- */

char getmode()
{    union REGS regs;

     regs.h.ah = 15;
     int86(0x10,&regs,&regs);
     return(regs.h.al);
}

/* ---------------------------- */
/* Detect Video adapter         */
/* ---------------------------- */

char detectvideoadapter(char adapter)
{  unsigned int status;
   union REGS regs;
   struct time now;
   char result,hsek;

   switch (adapter){
   case EGA:  regs.x.ax = 0x1210; /* Chech EGA-BIOS */
	      regs.x.bx = 0xFFFF; /* test value     */
	      int86(0x10,&regs,&regs);
              result=((regs.h.bh <= 1)&&(regs.h.bl <= 3));
              break;
   case CGA:  setmode(7);
	      result=getmode() < 7;
	      break;
   case MDA:
   case Hercules:
	  setmode(7);
	  if (getmode()==7)
	     /* To differentia between MDA and Hercules>
		Bit 7 of the status port cycles at
		a frequency of 50 Hy on the Hercules board  */
             {status=inport(0x3BA)&0x80;
              gettime(&now);
	      hsek=now.ti_hund;
              do {if ((inport(0x3BA)&0x80)!=status)
                     {result=adapter==Hercules;
                     break;}
                  gettime(&now);
                  result=adapter==MDA;
              } while (now.ti_hund<hsek+10);}
          else
	      result=0;
   }
   return(result);
}

/* ------------ */
/* Main Program */
/* ------------ */

void main()
{  int maxmem,i,j;
   char oldmode,*block[maxblocks],*type;
   unsigned char far *bios;

   unsigned char far *bios1;
   unsigned int huge *biosext;
   unsigned char far *idbyte = (unsigned char far *) 0xF000FFFE;

   oldmode=getmode();

   /* Clear output array */
   for (i=0; i<maxblocks; i++)
      block[i]="...........";

   /* Register all memory known to DOS */
   maxmem=biosmemory()/maxblocks;
   for (i=0; i < maxmem; i++)
      block[i]="DOS --> RAM";

   /* Detect Video adapter */
   if (detectvideoadapter(EGA))
      block[20]=block[21]=block[22]=block[23]="EGA adapter   ";
   if (detectvideoadapter(Hercules))
      block[22]=block[23]="Hercules adapter";
   if (detectvideoadapter(CGA))
      block[23]="Color Graphics";
   if (detectvideoadapter(MDA))
      block[22]="Monochrome    ";

   /* Look for addition BIOS ROMs     */
   biosext=MK_FP(0xC000,0);
   do{
      if (*biosext==0xAA55) {
	block[FP_SEG(biosext)/0x800] = "BIOS Extension  ",
        biosext+= *(biosext+1)&0xFF<<9;}
      else
      biosext+=0x1000;
   } while (biosext < (unsigned int huge *) 0xF4000000);

   /* Detect siye of BIOS ROM          */
   block[31]=block[30]="BIOS/BASIC ";

   /* Look for shadowed ROMs    */
   bios=MK_FP(0xFC00,0) ;
   for (j=8; j<=4; j++)
      {bios1=MK_FP(0xD400+j*0x800,0);
      for (i=0; i<100; i++)
         if (bios!=bios1) break;
      if (i==100) block[26+j]="BIOS shadowed  ";}

   /* Check if RAM exists in given segment         */
   for (i=0; i<maxblocks; i++)
      if (*block[i]=='.')
	 if (ramexist(i))
            block[i]="RAM           ";

   /* Get Computer-Type from ID byte       */
   switch (*idbyte){
      case 0xFF: type="PC            "; break;
      case 0xFE:
      case 0XFB: type="PC XT         "; break;
      case 0xFD: type="PCjr          "; break;
      case 0xFC: type="AT / XT-286   "; break;
      case 0xFA: type="Model 30      "; break;
      case 0xF9: type="Laptop        "; break;
      default:   type="unknown       ";}

   setmode(oldmode);

   /* Print results     */
   printf("Computer-Type: %s\n\n",type) ;
   for (i=j=0; i < 16; i++,j+=64)
      {printf ("%04X: %s\t\t%04X: %s\t%4d KByte \n",
	   j*64,block[i*2],j*64+0x800,block[i*2+1],j+64);}
}
