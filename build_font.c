#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (int argc,char **argv){
  unsigned char F[16],*p,l,ll;
  FILE *fp;
  unsigned long x,y,v,n,rx,ry,w,ox,r,ty,from_y,to_y,wx;

  ty=atol(argv[1]);
  from_y=atol(argv[2]);
  to_y=atol(argv[3]);
  wx=atol(argv[4]);
  
  printf("/* font %ld\n",ty);
  for(n=33;n<127;n++)printf("%c",(char)n);
  printf("\n*/\n");
  
  fp=fopen("my.ff","rb");
  fread(F,16,1,fp);
  if(memcmp(F,"farbfeld",8)!=0)exit(-1);
  x=F[8]<<24|F[9]<<16|F[10]<<8|F[11];
  y=F[12]<<24|F[13]<<16|F[14]<<8|F[15];
  y--;
  v=x*y;
  printf("%ld %ld %ld\n",x,y,v);
  p=malloc(v*sizeof(char));
  for(n=0;n<v;n++){
    fread(F,8,1,fp);
    p[n]=((F[0]<<16|F[2]<<8|F[4])==0)?1:0;
  }
  fclose(fp);
  
  ox=0;
  r=33;
  printf("int font[%ld][128][%ld]={\n",ty,to_y-from_y+1);
  for(rx=0;rx<x;rx++){
    w=0;
    for(ry=from_y-1;ry<to_y;ry++)if(p[rx+ry*x]!=0)w++;
    l=rx-ox;
    if(w==0&&(wx>0&&l==wx||wx==0&&l>0)){
      printf("  { /* c=%ld,%c */\n",r,r);
      for(ry=from_y-1;ry<to_y;ry++){
        printf("    0b");
        for(n=0;n<l;n++)printf("%d",p[ox+n+ry*x]);
        for(n=l;n<12;n++)printf("0");
        ll=l<<4;
        for(n=12;n<16;n++){
          printf("%d",(ll&0x80)?1:0);
          ll<<=1;
        }
        if(ry==y-1)printf("\n");
        else printf(",\n");
      }
      if(rx==x-2)printf("  }\n");
      else printf("  },\n");
      ox=rx+1;
      r++;
    }
  }
  printf("};\n");

}
