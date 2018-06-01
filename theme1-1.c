#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<tiff-4.0.3/libtiff/tiffio.h>

#define Isize  512	//取り扱う画像のサイズX
#define Jsize  Isize	//取り扱う画像のサイズY
#define Bnum   9 	//ボタンの数
#define Xsize  Jsize*2+Right+5	//表示ウィンドウのサイズX
#define Ysize  Isize+5	//表示ウインドウのサイズY
#define Right  100	//表示ウィンドウ内の右側スペースサイズ
#define BS     100  	
#define Fcol   255|255<<8|255<<16	
#define Bcol   1	//ウィンドウの背景色

Display    *d;
Window     Rtw,W,W1,W2,Side,Bt[Bnum];
GC         Gc,GcW1,GcW2;
Visual     *Vis;
XEvent     Ev;
XImage     *ImageW1,*ImageW2;
unsigned long Dep;

unsigned char dat[Isize][Jsize];	//取り扱う画像データ格納用
unsigned char tiffdat[Isize][Jsize];	//tiff形式で保存する際の画像データ格納用
int buff[Isize*Jsize];	
unsigned char buffer[Isize*Jsize];

//表示画像をTIFF形式で保存する関数
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
	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, Isize);	//画像の幅
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, Jsize);	//画像の高さ
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);	//ピクセルの色深度（ビット数）
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);	//ピクセルあたりの色数
	//TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, Jsize);	//数行をひとまとめにして1ストリップと定義している場合

	TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	//画像の種別(0:mono,1:gray,2:RGB,3:index color,6:YCrCb)
	TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);	//このタグの数値が1（CONTIG）ならばBIP配列、2（SEPARATE）ならBIL配列

	TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);	//原点開始位置を指定
	//TIFFSetField(image, TIFFTAG_XRESOLUTION, 512.0);	//Xにおけるpixels/resolutionを意味する。実寸のサイズと画面上のサイズの比を指定dpi
	//TIFFSetField(image, TIFFTAG_YRESOLUTION, 512.0);
	//TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER);
  
	// Write the information to the file
	TIFFWriteEncodedStrip(image, 0, buffer, Isize * Jsize);	//画像データをTiff形式のファイルに保存

	// Close the file
	TIFFClose(image);	
}

//画像ファイル読み込み
void read_file()
{
	FILE  *fi; 
	char name[150];

	printf("File Name  : ");
	scanf("%s",name);
	if((fi=fopen(name,"r"))==NULL){
		printf("Read Error\n");
		exit(1);
	}
	fread(dat,1,Isize*Jsize,fi);
}

//ウィンドウに画像を表示（左側）
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
	//画像を表示
	XPutImage(d,W1,GcW1,ImageW1,0,0,0,0,Jsize,Isize);
}

//ウィンドウに画像を表示（右側）
void view_imgW2(unsigned char ttt[Isize][Jsize])
{
	int i,j,k;
	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buff[k]=ttt[i][j]|ttt[i][j]<<8|ttt[i][j]<<16;
			tiffdat[i][j] = ttt[i][j];	//右側ウィンドウに表示された画像を保存するために格納
				k++;
		}
	}
	//画像を表示
	XPutImage(d,W2,GcW2,ImageW2,0,0,0,0,Jsize,Isize);
}

//階調変換の関数
void change_step(unsigned char dat[Isize][Jsize]){
	int i, j, s, v;
	unsigned char c, dat1[Isize][Jsize];
	printf("Number of steps : "); //階調数の入力
	scanf("%d",&s);
	v = 256/s;

	for(i=0; i<Isize; i++){
		for(j=0;j<Jsize;j++){
			c=(unsigned char)((dat[i][j]/256.)*s);
			dat1[i][j]=c*v;
		}
	}
	view_imgW2(dat1);//右側ウィンドウに画像を表示する関数を呼び出し
}

void noudo_henkan(unsigned char dat[Isize][Jsize]){
	int i,j;
	unsigned char max, min, dat2[Isize][Jsize];

	max = min = dat[0][0];
	for(i = 0; i<Isize; i++){
		for(j=0; j<Jsize; j++){
			if(dat[i][j]>max)max = dat[i][j];
			if(dat[i][j]<min)min = dat[i][j];
		}
	}
	for(i = 0; i<Isize; i++){
		for(j=0; j<Jsize; j++){
			if(dat[i][j]<min) dat2[i][j]=0;
			else if(dat[i][j]>max) dat2[i][j]=255;
			else{
				dat2[i][j]=(unsigned char)((dat[i][j]-min)*255.0/(max-min));
			}
		}
	}	
	view_imgW2(dat2);
}

void noudo_henkan2(unsigned char dat[Isize][Jsize]){
	int i,j;
	unsigned char max,min,dat2[Isize][Jsize];

	max = min = dat[0][0];
	for(i = 0; i<Isize; i++){
		for(j=0; j<Jsize; j++){
			if(dat[i][j]>max)max = dat[i][j];
			if(dat[i][j]<min)min = dat[i][j];
		}
	}
	for(i = 0; i<Isize; i++){
		for(j=0; j<Jsize; j++){
			if(dat[i][j]<min) dat2[i][j]=0;
			else if(dat[i][j]>max) dat2[i][j]=255;
			else{
				dat2[i][j]=(unsigned char)(pow((float)(dat[i][j]-min)/(float)(max-min),0.5)*(max-min));
			}
		}
	}	
	view_imgW2(dat2);
}

void histgram(unsigned char dat[Isize][Jsize]){
	const int LEVEL = 255;
	int i,j,hist[LEVEL];

	for (i = 0;i < LEVEL;i++)hist[i]=0;
	for(i = 0; i<Isize; i++){
		for(j=0; j<Jsize; j++){
			hist[dat[i][j]-'0']++;
		}
	}
	for (i = 0;i < LEVEL;i++)printf("%d : %d\n",i,hist[i]);
}

//windowの初期設定
void init_window()
{
	int i;

	//windowを開く前の初期設定
	//d=XOpenDisplay(NULL);	
  	// Xサーバとの接続
	if( (d = XOpenDisplay( NULL )) == NULL ) {
		fprintf( stderr, "Ｘサーバに接続できません\n" );
		exit(1);
	}
	
	// ディスプレイ変数の取得
	Rtw=RootWindow(d,0);	//ルートウィンドウを指定
	Vis=XDefaultVisual(d,0);
	Dep=XDefaultDepth(d,0);

	//windowを作成
	W=XCreateSimpleWindow(d,Rtw,0,0,Xsize,Ysize,2,Fcol,Bcol);	//背景ウィンドウ
	W1=XCreateSimpleWindow(d,W,0,0,Jsize,Isize,2,Fcol,Bcol);	//画像表示用ウィンドウ（左側）
	W2=XCreateSimpleWindow(d,W,Jsize,0,Jsize,Isize,2,Fcol,Bcol);	//画像表示用ウィンドウ（右側）
	Side=XCreateSimpleWindow(d,W,Jsize*2+5,0,Right,Isize,2,Fcol,Bcol);	//サイドウィンドウ
	for(i=0;i<Bnum;i++){
		Bt[i]=XCreateSimpleWindow(d,Side,0,30*i,BS,30,2,Fcol,Bcol);	//ボタン作成
		XSelectInput(d,Bt[i],ExposureMask | ButtonPressMask);	//ウィンドウが表示された時orボタンが押された時にXサーバから通知
	}

	XSelectInput(d,W1,ButtonPressMask);	
	XSelectInput(d,W2,ButtonPressMask);	

	//ウィンドウを画面に表示
	XMapWindow(d,W);
	XMapSubwindows(d,W1);
	XMapSubwindows(d,W2);
	XMapSubwindows(d,Side);
	XMapSubwindows(d,W);
}

//表示画像の初期設定
void init_image()
{
	//デフォルトのグラフィックスコンテキストを生成
	Gc  = XCreateGC(d,W,0,0);
	GcW1= XCreateGC(d,W1,0,0);
	GcW2= XCreateGC(d,W1,0,0);
  
	//表示画像の設定
	ImageW1=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,Jsize,Isize,
			BitmapPad(d),0);
	ImageW1->data = (char *)buff;

	ImageW2=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,Jsize,Isize,
			BitmapPad(d),0);
	ImageW2->data = (char *)buff;
}

//イベント発生用関数
void event_select()	
{
	int x,y;
	while(1){
		//イベント読み込み
		XNextEvent(d,&Ev);
		switch(Ev.type){
			//ウィンドウが表示された場合
			case Expose :
				XSetForeground(d,Gc,Fcol);	//前景色の設定
				XSetBackground(d,Gc,10);	//背景色の設定
				XDrawImageString(d,Bt[0],Gc,28,21,"Load",4);	//ボタンへ文字列を描画
				XDrawImageString(d,Bt[1],Gc,28,21,"ViewW1",6);
				XDrawImageString(d,Bt[2],Gc,28,21,"ViewW2",6);
				XDrawImageString(d,Bt[3],Gc,28,21,"Save",4);
				XDrawImageString(d,Bt[4],Gc,28,21,"step",4);
				XDrawImageString(d,Bt[5],Gc,28,21,"noudo",5);
				XDrawImageString(d,Bt[6],Gc,28,21,"noudo2",6);
				XDrawImageString(d,Bt[7],Gc,28,21,"histgram",8);
				XDrawImageString(d,Bt[Bnum-1],Gc,28,21,"Quit",4);
			break;
			//ボタンが押された場合
			case ButtonPress :
				if(Ev.xany.window == Bt[0]){
					read_file();
				}
				if(Ev.xany.window == Bt[1]){
					view_imgW1(dat);
				}
				if(Ev.xany.window == Bt[2]){
					view_imgW2(dat);
				}
				if(Ev.xany.window == Bt[3]){
					tiff_save(tiffdat);        
                }
				if(Ev.xany.window == Bt[4]){
					change_step(dat);        
                }
				if(Ev.xany.window == Bt[5]){
					noudo_henkan(dat);        
                }
				if(Ev.xany.window == Bt[6]){
					noudo_henkan2(dat);        
                }
				if(Ev.xany.window == Bt[7]){
					histgram(dat);        
                }
				if(Ev.xany.window == Bt[Bnum-1]){
					exit(1);
				}
				//押されたピクセルの座標と輝度を表示
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
	init_window();	
	init_image();
	event_select();
	return 0;
}
