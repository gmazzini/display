#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (int argc,char **argv){
  unsigned char F[16],*p,l;
  int *kk;
  FILE *fp;
  unsigned long x,y,v,n,rx,ry,w,r,ty,from_y,to_y,wx;

  // ty from_y to_y wx name.ff
  ty=atol(argv[1]);
  from_y=atol(argv[2]);
  to_y=atol(argv[3]);
  wx=atol(argv[4]);

  // read ff file with !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
  fp=fopen(argv[5],"rb");
  fread(F,16,1,fp);
  if(memcmp(F,"farbfeld",8)!=0)exit(-1);
  x=F[8]<<24|F[9]<<16|F[10]<<8|F[11];
  y=F[12]<<24|F[13]<<16|F[14]<<8|F[15];
  y--;
  v=x*y;
  p=malloc(v*sizeof(char));
  for(n=0;n<v;n++){
    fread(F,8,1,fp);
    p[n]=((F[0]<<16|F[2]<<8|F[4])==0)?1:0;
  }
  fclose(fp);
  kk=malloc(x*sizeof(int));

  l=to_y-from_y+1;
  printf("unsigned int font_%ld[97][%d]={\n",ty,l+1);

  // header
  printf("  { /* header */\n");
  printf("    %ld, /* ty */\n",ty);
  printf("    %d, /* y */\n",l);
  printf("    %ld, /* wx */\n",wx);
  for(n=3;n<=l;n++){
    if(n==l)printf("    0\n"); 
    else printf("    0,\n");
  }
  printf("  },\n");

  // space
  printf("  { /* c=space */\n");
  l=(wx==0)?1:wx;
  printf("    %d,\n",l);
  for(ry=from_y-1;ry<to_y;ry++){
    printf("    0b");
    for(n=0;n<16;n++)printf("0");
    if(ry==y-1)printf("\n");
    else printf(",\n");
  }
  printf("  },\n");

  // looking for vertical blanks
  for(rx=0;rx<x;rx++){
    w=0;
    for(ry=from_y-1;ry<to_y;ry++)if(p[rx+ry*x]!=0)w++;
    kk[rx]=w;
  }
  for(rx=0;rx<x;rx++){
    if(kk[rx]>0){
      w=1;
      for(n=rx+1;n<x;n++){
        if(kk[n]>0){
          kk[n]=-1;
          w++;
        }
        else break;
      }
      kk[rx]=w;
    }
  }
  
  // double apex processing
  l=0;
  for(rx=0;rx<x;rx++){
    if(kk[rx]>0&&l==1)break;
    if(kk[rx]>0)l++;
  }
  l=rx;
  for(rx++;rx<x;rx++)if(kk[rx]==0)break;
  for(rx++;rx<x;rx++)if(kk[rx]>0)break;
  for(rx++;rx<x;rx++)if(kk[rx]==0)break;
  kk[l]=rx-l;
  for(n=l+1;n<rx;n++)kk[n]=-1;

  // character processing
  r=33;
  for(rx=0;rx<x;rx++){
    if( wx>0 && rx%(wx+1)==0 && r<127 || wx==0 && kk[rx]>0 && r<127 ){
      if(wx==0)l=kk[rx];
      else l=wx;
      printf("  { /* c=%ld,%c */\n",r,(char )r);
      printf("    %d,\n",l);
      for(ry=from_y-1;ry<to_y;ry++){
        printf("    0b");
        for(n=0;n<l;n++)printf("%d",p[rx+n+ry*x]);
        for(n=l;n<16;n++)printf("0");
        if(ry==to_y-1)printf("\n");
        else printf(",\n");
      }
      if(r<126)printf("  },\n");
      else printf("  }\n");
      r++;
    }
  }
  printf("};\n");

}
