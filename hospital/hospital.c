#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h> 
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/input.h>   
#include <assert.h>    
#include <getopt.h>             /* getopt_long() */    
#include <fcntl.h>              /* low-level i/o */    
#include <unistd.h>             /*getpid()*/
#include <error.h>    
#include <malloc.h>    
#include <sys/mman.h>    
#include <linux/fb.h> 
#include "ASC16.h" 
#include "HZK16.h" 
// #include "concatenated_output3.h" 

#define COMMAND0 0
#define COMMAND1 1

unsigned char buf1[]="��һ����";
unsigned char buf2[]="�ڶ�����";
unsigned char buf3[]="�ȴ�����";

unsigned char ch[][16] = {	
	
{0x20,0x40,0x3F,0x7E,0x48,0x90,0x85,0x08,0x3F,0xF8,0x01,0x08,0x01,0x08,0x3F,0xF8},
{0x21,0x00,0x21,0x00,0x3F,0xFC,0x03,0x04,0x05,0x04,0x19,0x28,0xE1,0x10,0x01,0x00},/*"��",0*/
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"һ",1*/
{0x00,0x40,0x40,0x40,0x20,0xA0,0x21,0x10,0x02,0x08,0x04,0x26,0xE0,0x40,0x20,0x80},
{0x23,0x10,0x20,0x20,0x20,0x40,0x28,0x88,0x33,0x10,0x20,0x20,0x00,0xC0,0x07,0x00},/*"��",2*/
{0x02,0x00,0x01,0x00,0x7F,0xFE,0x40,0x02,0x80,0x04,0x3F,0xF8,0x04,0x00,0x08,0x20},
{0x1F,0xF0,0x01,0x10,0x01,0x00,0x3F,0xF8,0x01,0x00,0x01,0x00,0xFF,0xFE,0x00,0x00},/*"��",3*/
{0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00},/*"��",4*/

};

/**************оƬ��������*********************/
unsigned char SYN_StopCom[]={0xFD,0X00,0X02,0X02,0XFD};//ֹͣ�ϳ�
unsigned char SYN_SuspendCom[]={0XFD,0X00,0X02,0X03,0XFC};//��ͣ�ϳ�
unsigned char SYN_RecoverCom[]={0XFD,0X00,0X02,0X04,0XFB};//�ָ��ϳ�
unsigned char SYN_ChackCom[]={0XFD,0X00,0X02,0X21,0XDE};//״̬��ѯ
unsigned char SYN_PowerDownCom[]={0XFD,0X00,0X02,0X88,0X77};//����POWER DOWN ״̬����
/***********************************************/

unsigned int *lcdbuf=NULL;  


// ���ô��ڲ���
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop) {
    struct termios newtio, oldtio;
    if (tcgetattr(fd, &oldtio) != 0) { // ��ȡ��ǰ���ڲ���        
		perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag |= CLOCAL | CREAD; // �������Ӻͽ���ʹ��

    newtio.c_cflag &= ~CSIZE; // �������λ����

    switch (nBits) {
        case 5:
            newtio.c_cflag |= CS5;
            break;
        case 6:
            newtio.c_cflag |= CS6;
            break;
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch (nEvent) {
        case 'O': // ��У��            
			newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E': // żУ��            
			newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N': // ��У��
            newtio.c_cflag &= ~PARENB;
            newtio.c_iflag &= ~INPCK;
            newtio.c_iflag &= ~ISTRIP;
            break;
    }


	switch( nEvent )
	{
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E': 
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':  
		newtio.c_cflag &= ~PARENB;
		newtio.c_iflag &= ~INPCK;
		newtio.c_iflag &= ~ISTRIP;
		break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB; 
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
		
		newtio.c_cc[VTIME]  = 0;
		newtio.c_cc[VMIN] = 0;
		
		tcflush(fd,TCIOFLUSH);
		
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	
	//	printf("set done!\n\r");
	return 0;
}

void SYN_FrameInfo(int fd, unsigned char Music,unsigned char *HZdata) {
/****************��Ҫ���͵��ı�**********************************/ 
		 unsigned  char  Frame_Info[50];
         unsigned  char  HZ_Length;  
		 unsigned  char  ecc  = 0;  			//����У���ֽ�
	     unsigned  int i=0; 
		 HZ_Length =strlen(HZdata); 			//��Ҫ�����ı��ĳ���
 
/*****************֡�̶�������Ϣ**************************************/           
		 Frame_Info[0] = 0xFD ; 			//����֡ͷFD
		 Frame_Info[1] = 0x00 ; 			//�������������ȵĸ��ֽ�
		 Frame_Info[2] = HZ_Length + 3; 		//�������������ȵĵ��ֽ�
		 Frame_Info[3] = 0x01 ; 			//���������֣��ϳɲ�������		 		 
		 Frame_Info[4] = 0x01 | Music<<4 ;  //����������������������趨

/*******************У�������***************************************/		 
		 for(i = 0; i<5; i++)   				//���η��͹���õ�5��֡ͷ�ֽ�
	     {  
	         ecc=ecc^(Frame_Info[i]);		//�Է��͵��ֽڽ������У��	
	     }

	   	 for(i= 0; i<HZ_Length; i++)   		//���η��ʹ��ϳɵ��ı�����
	     {  
	         ecc=ecc^(HZdata[i]); 				//�Է��͵��ֽڽ������У��		
	     }		 
/*******************����֡��Ϣ***************************************/		  
		  memcpy(&Frame_Info[5], HZdata, HZ_Length);
		  Frame_Info[5+HZ_Length]=ecc;
		  write(fd,Frame_Info, 5+HZ_Length+1);
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


void draw32x32(int x,int y,unsigned char ch[], int color) {
	int i, j;
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			// ���� 
			if(ch[2 * i] & (1 << (7 - j))) {
				lcdbuf[(y + i * 2) * 480 + (x + j * 2)] = color;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + j * 2)] = color;
				lcdbuf[(y + i * 2) * 480 + (x + j * 2 + 1)] = color;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + j * 2 + 1)] = color;
			} else {
				lcdbuf[(y + i * 2) * 480 + (x + j * 2)] = 0x00;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + j * 2)] = 0x00;
				lcdbuf[(y + i * 2) * 480 + (x + j * 2 + 1)] = 0x00;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + j * 2 + 1)] = 0x00;
			}
			
			if(ch[2 * i + 1] & (1 << (7 - j))) {
				lcdbuf[(y + i * 2) * 480 + (x + 16 + j * 2    )] = color;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + 16 + j * 2)] = color;
				lcdbuf[(y + i * 2) * 480 + (x + 16 + j * 2 + 1)] = color;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + 17 + j * 2)] = color;
			} else {
				lcdbuf[(y + i * 2) * 480 + (x + 16 + j * 2    )] = 0x00;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + 16 + j * 2)] = 0x00;
				lcdbuf[(y + i * 2) * 480 + (x + 16 + j * 2 + 1)] = 0x00;
				lcdbuf[(y + i * 2 + 1) * 480 + (x + 17 + j * 2)] = 0x00;
			}	
		}
	} 
}


void drawnum1(int x,int y,unsigned char buf[], int color) {
	int i, t_x, t_y, index;
	for (i = 0; i < 10; i++) {
		t_x = x + i * 8 + 3;
		t_y = y + 14;
		if (buf[i] == '\n' || buf[i] == '\0' || buf[i] == 0) {
    		break;
		}
		if (buf[i] <= 0x7F) {
            // yingwen
            index = buf[i] * 16;
            draw8x16(t_x, t_y, &szASC16[index], color);
        } else {
            // zhongwen 
            index = (buf[i] - 161) * 94 + (buf[i + 1] - 161);
            index = index * 32;
            draw16x16(t_x, t_y, &szHZK16[index], color);
            i++;  
        }
	}

}



void drawclinic(int x,int y, int t, int color) {	// t = 1 | 2
	int i, t_x, t_y;
	for (i = 0; i < 4; i++) {
		t_x = x + i * 34 + 4;
		t_y = y + 14;
		if (i != 1 || t == 1) draw32x32(t_x, t_y, ch[i * 2], color);
		else draw32x32(t_x, t_y, ch[4 * 2], color);
	}
}

void drawclinic1(int x,int y, int color) {
	int i, t_x, t_y, index;
	for (i = 0; i < 4; i++) {
		t_x = x + i * 34 + 4;
		t_y = y + 14;
		index = ((buf1[i * 2]-161)*94 + (buf1[2 * i + 1]-161)) * 32;
		draw32x32(t_x, t_y, &szHZK16[index], color);
	}
}

void drawclinic2(int x,int y, int color) {
	int i, t_x, t_y, index;
	for (i = 0; i < 4; i++) {
		t_x = x + i * 34 + 4;
		t_y = y + 14;
		index = ((buf2[i * 2]-161)*94 + (buf2[2 * i + 1]-161)) * 32;
		draw32x32(t_x, t_y, &szHZK16[index], color);
	}
}

void drawclinic3(int x,int y, int color) {
	int i, t_x, t_y, index;
	for (i = 0; i < 4; i++) {
		t_x = x + i * 39 + 4;
		t_y = y + 14;
		index = ((buf3[i * 2]-161)*94 + (buf3[2 * i + 1]-161)) * 32;
		draw32x32(t_x, t_y, &szHZK16[index], color);
	}
}


void drawframe(int x, int y, int width, int height, int color) {
    int i, j;
    // ���ϱ߿�
    for (i = x; i < x + width; i++) {
        lcdbuf[y * 480 + i] = color;
    }
    // ���±߿�
    for (i = x; i < x + width; i++) {
        lcdbuf[(y + height - 1) * 480 + i] = color;
    }
    // ����߿�
    for (j = y; j < y + height; j++) {
        lcdbuf[j * 480 + x] = color;
    }
    // ���ұ߿�
    for (j = y; j < y + height; j++) {
        lcdbuf[j * 480 + x + width - 1] = color;
    }

}

void drawline1x3(int x1, int y1, int x2, int y2, int color) {
	int i, j;
	for (i = x1; i <= x2; i++) {
		for (j = y1; j <= y2; j++) {
			if((i + j) % 3 != 0)
				lcdbuf[j * 480 + i] = color;
			else 
				lcdbuf[j * 480 + i] = 0x000000;
		}
	}
}

void drawline1x1(int x1, int y1, int x2, int y2, int color) {
	int i, j;
	for (i = x1; i <= x2; i++) {
		for (j = y1; j <= y2; j++) {
				lcdbuf[j * 480 + i] = color;
		}
	}
}

void drawdev1(int x, int y, int width, int height, int color) {
	drawline1x3(x - 2, y - 2, x + width, y, color);
	drawline1x3(x - 2, y - 2, x, y + height, color);
	drawline1x3(x, y + height, x + width + 2, y + height + 2, color);
	drawline1x3(x + width, y, x + width + 2, y + height + 2, color);
	
	drawline1x1(x, y + 40, x + width, y + 40, color);
	drawline1x1(x + 140, y + 40, x + 140, y + 240, color);
	
	drawline1x3(x, y + 90, x + 140, y + 90, color);
	drawline1x1(x, y + 140, x + 140, y + 140, color);
	drawline1x3(x, y + 190, x + 140, y + 190, color);
	
	drawclinic1(30, 56, 0xffffff);
    drawclinic2(30, 56 + 100, 0xffffff);
    drawclinic3(230, 50, 0xffffff);
}


int main() {
	
	/***************����*******************/ 
	int queue[10007] = {0};
	int head = 0, back = 0;
	int patient = 0;
	int busy[2] = {0};
	int count[2] = {0}; 
	/***************����*******************/ 
	
	/***************��Ƶ*******************/ 
	int spk_fd,nByte;
	char *uart0 = "/dev/ttySAC0"; // UART0�豸�ļ�·��
    char buffer[512];
    char *uart_out = "please input\r\n";
    memset(buffer, 0, sizeof(buffer));

    printf("Ready to Init Uart0!\n");
    
	
    // ��UART0�豸�ļ�    
	if ((spk_fd = open(uart0, O_RDWR|O_NOCTTY)) < 0) {
   	    printf("open %s is failed!", uart0);
    }

    printf("open %s is successfully!\n", uart0);
    set_opt(spk_fd, 9600, 8, 'N', 1); // ���ô��ڲ���
    /***************��Ƶ*******************/ 
	
	/***************����*******************/ 
	int keys_fd, led_fd;  
  	char ret[2];  
  	struct input_event t;  
  	int retval;
  
  	// �������豸�ļ� "/dev/input/event0"�������ļ�������
  	keys_fd = open("/dev/input/event0", O_RDONLY);  
  	if (keys_fd <= 0)  
  	{  
  	  	printf("open /dev/input/event0 device error!\n");  
  	  	return -1;  
  	} 
  	printf("open /dev/input/event0 device successfully!\n");

  	led_fd = open("/dev/leds", O_RDWR);
  	if (led_fd == -1)
  	{
  	  	perror("error open\n");
 	   	exit(-1);
  	}
  	printf("open /dev/leds successfully\n");
    /***************����*******************/ 
    
    /***************��Ļ*******************/ 
    int i, j, lcd_fd;					// ��Ļ�ļ������� 
   	struct fb_var_screeninfo vinfo;
   	struct fb_fix_screeninfo finfo;
   	__u8 *fb_buf;						// ��Ļ������ 
   	int fb_xres, fb_yres, fb_bpp;		// bits per pixel
   	__u32 screensize;

   	lcd_fd=open("/dev/fb0",O_RDWR);	
   	if(lcd_fd<0) {
     	printf("Error:cannot open framebuffer device!\n");
     	return -1;
   	}
   	//get fb_fix_screeninfo
   	if(ioctl(lcd_fd,FBIOGET_FSCREENINFO,&finfo)) {
     	printf("Error reading fixed information!\n");
     	return -1;
    }
  	//get fb_var_screeninfo
   	if(ioctl(lcd_fd,FBIOGET_VSCREENINFO,&vinfo)) {
     	printf("Error reading variable information!\n");
     	return -1;
   	}
  	printf("%dx,%dy,%dbpp\n",vinfo.xres,vinfo.yres,vinfo.bits_per_pixel);
  	fb_xres=vinfo.xres;
  	fb_yres=vinfo.yres;
  	fb_bpp=vinfo.bits_per_pixel;
  
   	screensize=vinfo.xres*vinfo.yres*vinfo.bits_per_pixel/8;
   	fb_buf=(char *)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
  	if((int)(fb_buf) == -1) {
 	    printf("Error:failed to map framebuffer device to memory!\n");
      	close(lcd_fd);
      	return -1;
   	}
  	memset(fb_buf,0,screensize); 
  	lcdbuf =(unsigned int*) fb_buf;
    /***************��Ļ*******************/ 
    
    /***************main*******************/ 
    
    
    /***************test*******************/ 
    drawdev1(30, 16, 420, 240, 0xffffff);
    drawnum1(30 + 8 * 3, 110, "003: ��*��", 0xffffff);
    drawnum1(30 + 8 * 3, 110 + 100, "005: ��*��", 0xffffff);
    
	 
    drawnum1(30 + 8 * 3 + 140, 110, "005: ��*��", 0xffffff);
    drawnum1(30 + 8 * 3 + 280, 110, "005: ��*��", 0xffffff);
    
    drawnum1(30 + 8 * 3 + 140, 110 + 35, "005: ��*��", 0xffffff);
    drawnum1(30 + 8 * 3 + 280, 110 + 35, "005: ��*��", 0xffffff);
    
    drawnum1(30 + 8 * 3 + 140, 110 + 70, "005: ��*��", 0xffffff);
    drawnum1(30 + 8 * 3 + 280, 110 + 70, "005: ��*��", 0xffffff);
    
    drawnum1(30 + 8 * 3 + 140, 110 + 105, "005: ��*��", 0xffffff);
    drawnum1(30 + 8 * 3 + 280, 110 + 105, "005: ��*��", 0xffffff);
    
    
    
    
    /*
    drawframe(30, 56, 420, 215, 0xffffff);
    drawframe(30, 106, 420, 1, 0xffffff);
    drawframe(30, 156, 420, 1, 0xffffff);
    drawframe(30, 206, 420, 1, 0xffffff);
    drawframe(170, 56, 1, 240, 0xffffff);
    */
    /***************test*******************/ 
    
    while (0) {
    // �������豸�ļ���ȡһ�������¼��ṹ�� input_event
	    if (read(keys_fd, &t, sizeof(t)) == sizeof(t)) {  
	      	if (t.type == EV_KEY) {
	        	if (t.value == 0 || t.value == 1) {  
	          		// ��ʾ���»��ͷŵİ����Լ���Ӧ�ļ�ֵ
	          		printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");  
	          		// �����������Ӧ�Ĵ����߼����� LED ������
	          		switch (t.code) {
	              		case 114: {
	              			
	              			if (t.value) {
							  	printf("114 key is pressed\t");
							  	printf("queue[%d] push %d\n", back, patient + 1);
	              				queue[back++] = ++patient;
	              				// ���������������������������и���ʾLCD���������� 
	              				
							}
							break;
						}
						case 116: {
							if (t.value) {
								printf("116 key is pressed\n");
								// �������������������������Ŷ������������� 
								
							}
							break;
						}
						case 172: {
							if (t.value) {
								printf("172 key is pressed\n");
								
								count[0]++;
								if (count[0] % 2 != 0) {
									// �ñ��˽��� 
									if(head == back) {
										busy[0] = 0;
										printf("doctor is eatting apple\n");
									} else {
										busy[0] = queue[head++];
										printf("queue[%d] pop %d\n", head, busy[0]);
									}
									
									if(busy[0]) {
										retval = ioctl(led_fd, COMMAND1, 0); // ���� 
										SYN_FrameInfo(spk_fd, 0x02, "һλ"); // ---------------��ʱ���Ż� 
									} else {
										retval = ioctl(led_fd, COMMAND0, 0); // ��� 
										SYN_FrameInfo(spk_fd, 0x02, "��һ"); // ---------------��ʱ���Ż�
										count[0]++;							 // һ�����ͳ�ȥ 
									}
								} else {
									// �ñ��˳�ȥ
									retval = ioctl(led_fd, COMMAND0, 0); // ��� 
									printf("binbinchuqu\n");
								}
							}
							break;
						}
						case 158: {
							if (t.value) {
								printf("158 key is pressed\n");
								count[1]++;
								if (count[1] % 2 != 0) {
									// �ñ��˽��� 
									if(head == back) {
										busy[1] = 0;
										printf("doctor is eatting apple\n");
									} else {
										busy[1] = queue[head++];
										printf("queue[%d] pop %d\n", head, busy[1]);
									}
									
									if(busy[1]) {
										retval = ioctl(led_fd, COMMAND1, 1); // ���� 
										SYN_FrameInfo(spk_fd, 0x02, "һλ"); // ---------------��ʱ���Ż� 
									} else {
										retval = ioctl(led_fd, COMMAND0, 1); // ��� 
										SYN_FrameInfo(spk_fd, 0x02, "��һ"); // ---------------��ʱ���Ż�
										count[1]++;							 // һ�����ͳ�ȥ 
									}
								} else {
									// �ñ��˳�ȥ
									retval = ioctl(led_fd, COMMAND0, 1); // ��� 
									printf("binbinchuqu\n");
								}
								
								
								
							}
							break;
						}
						default:
							break;
					}
	        	}
        	}
		}
    }
 	
  	close(keys_fd); 	// ����
  	close(lcd_fd);		// ��Ļ 
  	close(led_fd);		// �� 
  	close(spk_fd);		// ��Ƶ

    /***************main*******************/
    
    
    
    
    
    /*
		while(1){
			SYN_FrameInfo(fd, 0x02, "���Ϻ�BYD");
			sleep(10);
		} 
	*/ 
    
    
}







