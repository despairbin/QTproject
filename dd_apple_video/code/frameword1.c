#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <assert.h>    

#include <getopt.h>             /* getopt_long() */    

#include <fcntl.h>              /* low-level i/o */    
#include <unistd.h>             /*getpid()*/
#include <error.h>    
#include <errno.h>
#include <malloc.h>    
#include <sys/stat.h>    
#include <sys/types.h>    
#include <sys/time.h>    
#include <sys/mman.h>    
#include <sys/ioctl.h>
#include <linux/fb.h> 
#include "ASC16.h" 
#include "concatenated_output3.h" 
unsigned int *lcdbuf=NULL;  
unsigned char ch[][16] = {
{0x00,0x10,0x20,0x14,0x11,0xD2,0x00,0x12,0xFC,0x10,0x0B,0xFE,0x08,0x10,0x48,0x90},
{0x28,0x90,0x12,0xD0,0x12,0x90,0x2A,0x90,0x2A,0x8A,0x4A,0xEA,0x87,0x06,0x02,0x02},/*"±Û",0*/
{0x00,0x10,0x20,0x14,0x11,0xD2,0x00,0x12,0xFC,0x10,0x0B,0xFE,0x08,0x10,0x48,0x90},
{0x28,0x90,0x12,0xD0,0x12,0x90,0x2A,0x90,0x2A,0x8A,0x4A,0xEA,0x87,0x06,0x02,0x02},/*"±Û",1*/
{0x02,0x00,0x02,0x00,0xFF,0xFE,0x04,0x00,0x04,0x00,0x0F,0xF0,0x08,0x10,0x18,0x10},
{0x2F,0xF0,0x48,0x10,0x88,0x10,0x0F,0xF0,0x08,0x10,0x08,0x10,0x08,0x50,0x08,0x20},/*"”–",2*/
{0x00,0x00,0x01,0xFC,0xFD,0x24,0x11,0x24,0x11,0xFC,0x11,0x24,0x11,0x24,0x7D,0xFC},
{0x10,0x20,0x10,0x20,0x11,0xFC,0x10,0x20,0x1C,0x20,0xE0,0x20,0x43,0xFE,0x00,0x00},/*"¿Ì",3*/

};
unsigned char ch0[]={0x00,0x00,0x00,0x00,0x78,0x44,0x44,0x78,0x46,0x42,0x42,0x7C,0x00,0x00,0x00,0x00};/*"B",0*/
unsigned char ch1[]={0x00,0x00,0x00,0x00,0x82,0x44,0x28,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x00};/*"Y",1*/
unsigned char ch2[]={0x00,0x00,0x00,0x00,0x78,0x46,0x42,0x41,0x41,0x43,0x46,0x7C,0x00,0x00,0x00,0x00};/*"D",2*/




void draw8x16(int x,int y,unsigned char ch[],int color); 
void draw16x24(int x,int y,unsigned char ch[],int color); 
void draw320x240(int x,int y,unsigned char ch[], int color);

int main(int argc,char *argv[])
{
   int i,j,fd,fbfd;
   struct fb_var_screeninfo vinfo;
   struct fb_fix_screeninfo finfo;
   __u8 *fb_buf;
   int fb_xres,fb_yres,fb_bpp;//bits per pixel
   __u32 screensize;

   fbfd=open("/dev/fb0",O_RDWR);
   if(fbfd<0)
   {
     printf("Error:cannot open framebuffer device!\n");
     return -1;
   }
   //get fb_fix_screeninfo
   if(ioctl(fbfd,FBIOGET_FSCREENINFO,&finfo))
   {
     printf("Error reading fixed information!\n");
     return -1;
   }
  //get fb_var_screeninfo
   if(ioctl(fbfd,FBIOGET_VSCREENINFO,&vinfo))
   {
     printf("Error reading variable information!\n");
     return -1;
   }
  printf("%dx,%dy,%dbpp\n",vinfo.xres,vinfo.yres,vinfo.bits_per_pixel);
  fb_xres=vinfo.xres;
  fb_yres=vinfo.yres;
  fb_bpp=vinfo.bits_per_pixel;
  
   screensize=vinfo.xres*vinfo.yres*vinfo.bits_per_pixel/8;
   fb_buf=(char *)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fbfd,0);
   if((int)(fb_buf)==-1)
   {
      printf("Error:failed to map framebuffer device to memory!\n");
      close(fbfd);
      return -1;
   }
  memset(fb_buf,0,screensize); 
  lcdbuf =(unsigned int*) fb_buf;
  //===================================================================
    /*draw16x16(100,100,ch[0], 0xff0000);
    draw16x16(120,100,ch[2], 0xff0000);
    draw16x16(140,100,ch[4], 0xff0000);
    draw16x16(160,100,ch[6], 0xff0000);*/
    
    for(i=0;i<6573;i++){
      		draw320x240(80,16,&ca[i*1200*8],0xffffff);
			usleep(1000*10/15);
    }
  //===================================================================
  
  
  printf("ummap framebuffer device to memory!\n");
  //sleep(5);
  munmap(fb_buf,screensize);
  close(fbfd);
  return 0;
 }  
void draw8x16(int x,int y,unsigned char ch[],int color)
{
  int i,j;
  for (i=0;i<16;i++)
  {
     for(j=0;j<8;j++)
	 {
		if(ch[i]&(1<<(7-j))) 
		   {
			  lcdbuf[(y+i)*480+(x+j)] = color;
		     }
		else{
			  lcdbuf[(y+i)*480+(x+j)] = 0x00;
		     }
	 }
  }	  
}

void draw16x16(int x,int y,unsigned char ch[],int color)
{
  int i,j;
  for (i=0;i<16;i++)
  {
     for(j=0;j<8;j++)
	 {
		if(ch[2*i]&(1<<(7-j))) 
		   {
			  lcdbuf[(y+i)*480+(x+j)] = color;
		     }
		else{
			  lcdbuf[(y+i)*480+(x+j)] = 0x00;
		     }
	 }                                       //first byte
	 
	 
	 for(j=0;j<8;j++)
	 {
		if(ch[2*i+1]&(1<<(7-j))) 
		   {
			  lcdbuf[(y+i)*480+(x+8+j)] = color;
		     }
		else{
			  lcdbuf[(y+i)*480+(x+8+j)] = 0x00;
		     }
	 }                                        //second byte
	 	  
  }	  
}

void draw320x240(int x,int y,unsigned char ch[], int color) {
  int i, j, k;
  for (i = 0; i < 240; i++) {
  	for(k = 0; k < 40; k++) {
  		for (j = 0; j < 8; j++) {
      		if (ch[i * 40 + k] & (1 << (7 - j)))
      		  	lcdbuf[(y + i) * 480 + k * 8 + (x + j)] = color;
     		 else
    		    lcdbuf[(y + i) * 480 + k * 8 + (x + j)] = 0x00;
    	}
	}
    
  }
}



