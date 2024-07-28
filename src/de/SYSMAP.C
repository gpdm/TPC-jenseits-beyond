#include <stdio.h>
#include <bios.h>
#include <dos.h>

#define laenge 30
#define maxbloecke 32
#define CGA 1
#define MDA 2
#define Hercules 3
#define EGA 4

/* --------------------------------------------------- */
/* Test, ob in numerierten Speicherblock RAM vorhanden */
/* --------------------------------------------------- */

char ramexist (char blocknr)
{   unsigned int far *ram;
    unsigned int j,test;

    ram=MK_FP(blocknr*0x800,0);
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
/* Video-Modus setzen */
/* ------------------ */

void setmodus(char modus)
{  union REGS regs;

    regs.h.ah = 0;
    regs.h.al = modus;
    int86(0x10,&regs,&regs);
}

/* ------------------------------- */
/* Aktuellen Video-Modus ermitteln */
/* ------------------------------- */

char getmodus()
{    union REGS regs;

     regs.h.ah = 15;
     int86(0x10,&regs,&regs);
     return(regs.h.al);
}

/* ---------------------------- */
/* Bildschirm-Adapter ermitteln */
/* ---------------------------- */

char adaptervorhanden(char adapter)
{  unsigned int status;
   union REGS regs;
   struct time now;
   char result,hsek;

   switch (adapter){
   case EGA:  regs.x.ax = 0x1210; /* EGA-BIOS ansprechen */
	      regs.x.bx = 0xFFFF; /* Testwert */
	      int86(0x10,&regs,&regs);
              result=((regs.h.bh <= 1)&&(regs.h.bl <= 3));
              break;
   case CGA:  setmodus(7);
              result=getmodus() < 7;
	      break;
   case MDA:
   case Hercules:
	  setmodus(7);
	  if (getmodus()==7)
             /* Unterscheidung zwischen MDA und Hercules
		Bit 7 des Statusports aendert sich auf der
                Hercules-Karte mit einer Frequenz von 50 Hz */
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

/* ------------- */
/* Hauptprogramm */
/* ------------- */

void main()
{  int maxmem,i,j;
   char altmodus,*block[maxbloecke],*type;
   unsigned char far *bios;

   unsigned char far *bios1;
   unsigned int huge *biosext;
   unsigned char far *idbyte = (unsigned char far *) 0xF000FFFE;

   altmodus=getmodus();

   /* Ausgabe-Array loeschen */
   for (i=0; i<maxbloecke; i++)
      block[i]="...........";

   /* Fuer DOS bekannten Speicher eintragen */
   maxmem=biosmemory()/maxbloecke;
   for (i=0; i < maxmem; i++)
      block[i]="DOS --> RAM";

   /*Bildschirm-Adapter ermitteln */
   if (adaptervorhanden(EGA))
      block[20]=block[21]=block[22]=block[23]="EGA-Karte     ";
   if (adaptervorhanden(Hercules))
      block[22]=block[23]="Hercules-Karte";
   if (adaptervorhanden(CGA))
      block[23]="Farbgrafik    ";
   if (adaptervorhanden(MDA))
      block[22]="Monochrom     ";

   /* Zusaetzliche BIOS-EPROMs suchen */
   biosext=MK_FP(0xC000,0);
   do{
      if (*biosext==0xAA55) {
        block[FP_SEG(biosext)/0x800] = "BIOS-Erweiterung",
        biosext+= *(biosext+1)&0xFF<<9;}
      else
      biosext+=0x1000;
   } while (biosext < (unsigned int huge *) 0xF4000000);

   /* Groesse des BIOS-EPROMs ermitteln /
   block[31]=block[30]="BIOS/BASIC ";

   /* Gespiegelte EPROMs suchen */
   bios=MK_FP(0xFC00,0) ;
   for (j=8; j<=4; j++)
      {bios1=MK_FP(0xD400+j*0x800,0);
      for (i=0; i<100; i++)
         if (bios!=bios1) break;
      if (i==100) block[26+j]="BIOS gespiegelt";}

   /* In freien Bereichen testen, ob RAM vorhanden */
   for (i=0; i<maxbloecke; i++)
      if (*block[i]=='.')
	 if (ramexist(i))
            block[i]="RAM           ";

   /* Computer-Type aus ID-Byte ermitteln */
   switch (*idbyte){
      case 0xFF: type="PC            "; break;
      case 0xFE:
      case 0XFB: type="PC XT         "; break;
      case 0xFD: type="PCjr          "; break;
      case 0xFC: type="AT / XT-286   "; break;
      case 0xFA: type="Model 30      "; break;
      case 0xF9: type="Laptop        "; break;
      default:   type="unbekannt     ";}

   setmodus(altmodus);

   /* Ergebnis ausgeben */
   printf("Computer-Typ: %s\n\n",type) ;
   for (i=j=0; i < 16; i++,j+=64)
      {printf ("%04X: %s\t\t%04X: %s\t%4d KByte \n",
	   j*64,block[i*2],j*64+0x800,block[i*2+1],j+64);}
}
