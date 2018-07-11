#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<tiff-4.0.3/libtiff/tiffio.h>

#define Isize  512	//��갷�������Υ�����X
#define Jsize  Isize	//��갷�������Υ�����Y
#define Bnum   7 	//�ܥ���ο�
#define Xsize  Jsize*2+Right+5	//ɽ��������ɥ��Υ�����X
#define Ysize  Isize+5	//ɽ��������ɥ��Υ�����Y
#define Right  100	//ɽ��������ɥ���α�¦���ڡ���������
#define BS     100  	
#define Fcol   255|255<<8|255<<16	
#define Bcol   1	//������ɥ����طʿ�

Display    *d;
Window     Rtw,W,W1,W2,Side,Bt[Bnum];
GC         Gc,GcW1,GcW2;
Visual     *Vis;
XEvent     Ev;
XImage     *ImageW1,*ImageW2;
unsigned long Dep;

void init_window(),init_color(),init_image(),
	event_select();

unsigned char dat[Isize][Jsize];	//��갷�������ǡ�����Ǽ��
unsigned char tiffdat[Isize][Jsize];	//tiff��������¸����ݤβ����ǡ�����Ǽ��
int buff[Isize*Jsize];	
unsigned char buffer[Isize*Jsize];

unsigned char   image[Isize][Jsize];
unsigned char   bin[Isize][Jsize];

int buff[Isize*Jsize];
int flag;

			 
//ɽ��������TIFF��������¸����ؿ�
void tiff_save(unsigned char img[Isize][Jsize]){
	TIFF *image;
	
	int i,j,k;
	char save_fname[256];

	printf("Save file name (***.tiff) : ");
	scanf("%s",save_fname);
	
	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buffer[k]=img[i][j];
				k++;
		}
	}
	// Open the TIFF file
	if((image = TIFFOpen(save_fname, "w")) == NULL){
		printf("Could not open output file for writing\n");
		exit(42);
	}

	// We need to set some values for basic tags before we can add any data
	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, Isize);	//��������
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, Jsize);	//�����ι⤵
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);	//�ԥ�����ο����١ʥӥåȿ���
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);	//�ԥ����뤢����ο���
	//TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, Jsize);	//���Ԥ�ҤȤޤȤ�ˤ���1���ȥ�åפ�������Ƥ�����

	TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	//�����μ���(0:mono,1:gray,2:RGB,3:index color,6:YCrCb)
	TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);	//���Υ����ο��ͤ�1��CONTIG�ˤʤ��BIP����2��SEPARATE�ˤʤ�BIL����

	TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);	//�������ϰ��֤����
	//TIFFSetField(image, TIFFTAG_XRESOLUTION, 512.0);	//X�ˤ�����pixels/resolution���̣���롣�����Υ������Ȳ��̾�Υ�������������dpi
	//TIFFSetField(image, TIFFTAG_YRESOLUTION, 512.0);
	//TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER);
  
	// Write the information to the file
	TIFFWriteEncodedStrip(image, 0, buffer, Isize * Jsize);	//�����ǡ�����Tiff�����Υե��������¸

	// Close the file
	TIFFClose(image);	
}

//�����ե������ɤ߹���
void read_file()
{
	FILE  *fi; 
	char name[150];
	int i,j,k;
	printf("File Name  : ");
	scanf("%s",name);
	if((fi=fopen(name,"r"))==NULL){
		printf("Read Error\n");
		exit(1);
	}
	fread(dat,1,Isize*Jsize,fi);
}

//������ɥ��˲�����ɽ���ʺ�¦��
void view_imgW1(unsigned char ttt[Isize][Jsize])
{
	int i,j,k;
	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buff[k]=ttt[i][j]|ttt[i][j]<<8|ttt[i][j]<<16;
				k++;
		}
	}
	//������ɽ��
	XPutImage(d,W1,GcW1,ImageW1,0,0,0,0,Jsize,Isize);
}

//������ɥ��˲�����ɽ���ʱ�¦��
void view_imgW2(unsigned char ttt[Isize][Jsize])
{
	int i,j,k;
	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buff[k]=ttt[i][j]|ttt[i][j]<<8|ttt[i][j]<<16;
			tiffdat[i][j] = ttt[i][j];	//��¦������ɥ���ɽ�����줿��������¸���뤿��˳�Ǽ
				k++;
		}
	}
	//������ɽ��
	XPutImage(d,W2,GcW2,ImageW2,0,0,0,0,Jsize,Isize);
}

//���Ͳ��Τ�����������ؿ����������β����ΰ�ư��Ԥ�
void for_binary()
{
	int i,j;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			switch(flag){
				case 0:
					image[i][j]=dat[i][j];
				break;
			}
		}
	}
}

//���Ͳ���¹Ԥ���ؿ�
void binarization(int t)
{
	int i,j;

	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			if(image[i][j]<t) 
				bin[i][j]=0;	
			else
				bin[i][j]=255;	
		}
	}
	view_imgW2(bin);
}

void p_tail(){
    int i,j,k,S0,N0,N,t;
    float diff, buf, p;

    printf("S0 : ");
    scanf("%d",&S0);

    N=Isize*Jsize;
    p=S0/(float)N;
    diff=100000.;
    for(k=1;k<254;k++){
        N0=0;
        for(i=0;i<Isize;i++){
            for(j=0;j<Jsize;j++){
                if(image[i][j]>=k) N0++;
            }
        }
        buf=fabs(p-N0/(float)N);
        if(buf<diff){
            diff=buf;
            t=k;
        }
    }
    printf("p_tail threshold: %d\n",t);
    binarization(t);
}

//window�ν������
void init_window()
{
	int i;

	//window�򳫤����ν������
	//d=XOpenDisplay(NULL);	
  	// X�����ФȤ���³
	if( (d = XOpenDisplay( NULL )) == NULL ) {
		fprintf( stderr, "�إ����Ф���³�Ǥ��ޤ���\n" );
		exit(1);
	}
	
	// �ǥ����ץ쥤�ѿ��μ���
	Rtw=RootWindow(d,0);	//�롼�ȥ�����ɥ������
	Vis=XDefaultVisual(d,0);
	Dep=XDefaultDepth(d,0);

	//window�����
	W=XCreateSimpleWindow(d,Rtw,0,0,Xsize,Ysize,2,Fcol,Bcol);	//�طʥ�����ɥ�
	W1=XCreateSimpleWindow(d,W,0,0,Jsize,Isize,2,Fcol,Bcol);	//����ɽ���ѥ�����ɥ��ʺ�¦��
	W2=XCreateSimpleWindow(d,W,Jsize,0,Jsize,Isize,2,Fcol,Bcol);	//����ɽ���ѥ�����ɥ��ʱ�¦��
	Side=XCreateSimpleWindow(d,W,Jsize*2+5,0,Right,Isize,2,Fcol,Bcol);	//�����ɥ�����ɥ�
	for(i=0;i<Bnum;i++){
		Bt[i]=XCreateSimpleWindow(d,Side,0,30*i,BS,30,2,Fcol,Bcol);	//�ܥ������
		XSelectInput(d,Bt[i],ExposureMask | ButtonPressMask);	//������ɥ���ɽ�����줿��or�ܥ��󤬲����줿����X�����Ф�������
	}

	XSelectInput(d,W1,ButtonPressMask);	
	XSelectInput(d,W2,ButtonPressMask);	

	//������ɥ�����̤�ɽ��
	XMapWindow(d,W);
	XMapSubwindows(d,W1);
	XMapSubwindows(d,W2);
	XMapSubwindows(d,Side);
	XMapSubwindows(d,W);
}

//ɽ�������ν������
void init_image()
{
	//�ǥե���ȤΥ���ե��å�������ƥ����Ȥ�����
	Gc  = XCreateGC(d,W,0,0);
	GcW1= XCreateGC(d,W1,0,0);
	GcW2= XCreateGC(d,W1,0,0);
  
	//ɽ������������
	ImageW1=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,Jsize,Isize,
			BitmapPad(d),0);
	ImageW1->data = (char *)buff;

	ImageW2=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,Jsize,Isize,
			BitmapPad(d),0);
	ImageW2->data = (char *)buff;
}

//���٥��ȯ���Ѵؿ�
void event_select()
{
	int x,y,t;
	while(1){
		//���٥���ɤ߹���
		XNextEvent(d,&Ev);
		switch(Ev.type){
			//������ɥ���ɽ�����줿���
			case Expose :
				XSetForeground(d,Gc,Fcol);	//���ʿ�������
				XSetBackground(d,Gc,10);	//�طʿ�������
				XDrawImageString(d,Bt[0],Gc,28,21,"Load",4);	//�ܥ����ʸ���������
				XDrawImageString(d,Bt[1],Gc,28,21,"ViewW1",6);
				XDrawImageString(d,Bt[2],Gc,28,21,"ViewW2",6);
				XDrawImageString(d,Bt[3],Gc,28,21,"Save",4);
				XDrawImageString(d,Bt[4],Gc,28,21,"binary",6);
				XDrawImageString(d,Bt[5],Gc,28,21,"p_tail",6);
				XDrawImageString(d,Bt[Bnum-1],Gc,28,21,"Quit",4);
			break;
			case ButtonPress :
				if(Ev.xany.window == Bt[0]){
					read_file();
				}
				if(Ev.xany.window == Bt[1]){
					flag=0;
					view_imgW1(dat);
				}
				if(Ev.xany.window == Bt[2]){
					flag=0;
					view_imgW2(dat);
				}
				if(Ev.xany.window == Bt[3]){
					flag=0;
					tiff_save(tiffdat);
				}
				if(Ev.xany.window == Bt[4]){
					for_binary();
					printf("Threshold value : ");
					scanf("%d",&t);
					binarization(t);
				}
				if(Ev.xany.window == Bt[5]){
					p_tail();
                    view_imgW2(dat);
				}
				if(Ev.xany.window == Bt[Bnum-1]){
					exit(1);
				}
				if(Ev.xany.window == W1){
					x=Ev.xbutton.x; y=Ev.xbutton.y;
					printf("(%d %d) %d\n",y,x,dat[y][x]);
				}
				if(Ev.xany.window == W2){
					x=Ev.xbutton.x; y=Ev.xbutton.y;
					printf("(%d %d) %d\n",y,x,dat[y][x]);
				}
			break;
		}
	}
}


int main()
{
	flag=0;
	init_window();
	init_image();
	event_select();
	return 0;
}

