/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.44                          *
*        Compiled Nov 10 2017, 08:53:57                              *
*        (c) 2017 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/
#include <stddef.h>
#include <stdio.h>
#include <string.h>
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
/* STemWIN头文件 */
#include "ScreenShot.h"
#include "MainTask.h"
#include "usart/bsp_usart.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Types
*
**********************************************************************
*/


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
int t0, t1;
static char _acBuffer[1024 * 4];
static char *_acbuffer = NULL;

UINT    f_num;
extern FATFS   fs;								/* FatFs文件系统对象 */
extern FIL     file;							/* file objects */
extern FRESULT result; 
extern DIR     dir;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/**
  * @brief 从存储器中读取数据
  * @note 无
  * @param 
  * @retval NumBytesRead：读到的字节数
  */
int _GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off)
{
	static int FileAddress = 0;
	UINT NumBytesRead;
	FIL *Picfile;
	
	Picfile = (FIL *)p;
	
  /* 检查缓冲区大小 */
	if(NumBytesReq > sizeof(_acBuffer))
	{NumBytesReq = sizeof(_acBuffer);}
  
	/* 读取偏移量 */
	if(Off == 1) FileAddress = 0;
	else FileAddress = Off;
	result = f_lseek(Picfile, FileAddress);
	
	/* 进入临界段 */
	taskENTER_CRITICAL();
  /* 读取图片数据 */
	result = f_read(Picfile, _acBuffer, NumBytesReq, &NumBytesRead);
	/* 退出临界段 */
	taskEXIT_CRITICAL();
	
	*ppData = (const U8 *)_acBuffer;
	
  /* 返回以读到的字节数 */
	return NumBytesRead;
}

/**
  * @brief 直接从外部存储器中绘制BMP图片
  * @note 无
  * @param sFilename：需要加载的图片名
  *        x0：图片左上角在屏幕上的横坐标
  *        y0：图片左上角在屏幕上的纵坐标
  * @retval 无
  */
static void ShowBMPEx(const char *sFilename, int x0, int y0)
{
	/* 进入临界段 */
	taskENTER_CRITICAL();
	/* 打开图片 */
	result = f_open(&file, sFilename, FA_READ);
	if ((result != FR_OK))
	{
		printf("文件打开失败！\r\n");
	}
  /* 退出临界段 */
	taskEXIT_CRITICAL();
  
  /* 绘制图片 */
  GUI_BMP_DrawEx(_GetData, &file, x0, y0);
}

/**
  * @brief 加载BMP图片到内存中并绘制
  * @note 无
  * @param sFilename：需要加载的图片名
  *        x0：图片左上角在屏幕上的横坐标
  *        y0：图片左上角在屏幕上的纵坐标
  * @retval 无
  */
static void ShowBMP(const char *sFilename, int x0, int y0)
{
	WM_HMEM hMem;
	
	/* 进入临界段 */
	taskENTER_CRITICAL();
	/* 打开图片 */
	result = f_open(&file, sFilename, FA_READ);
	if ((result != FR_OK))
	{
		printf("文件打开失败！\r\n");
		_acbuffer[0]='\0';
	}
	
	/* 申请一块动态内存空间 */
	hMem = GUI_ALLOC_AllocZero(file.fsize);
	/* 转换动态内存的句柄为指针 */
	_acbuffer = GUI_ALLOC_h2p(hMem);

	/* 读取图片数据到动态内存中 */
	result = f_read(&file, _acbuffer, file.fsize, &f_num);
	if(result != FR_OK)
	{
		printf("文件读取失败！\r\n");
	}
	/* 读取完毕关闭文件 */
	f_close(&file);
	/* 退出临界段 */
	taskEXIT_CRITICAL();
	
	GUI_BMP_Draw(_acbuffer, x0, y0);
  
	/* 释放内存 */
	GUI_ALLOC_Free(hMem);
}

/**
  * @brief 加载BMP图片数据到内存设备并绘制
  * @note 无
  * @param sFilename：需要加载的图片名
  *        ScaledMode：是否启用缩放，0不启用，1启用
  *        Num：缩放系数的分子
  *        Denom：缩放系数的分母
  * @retval 无
  */
WM_HMEM hMem;
GUI_MEMDEV_Handle hBMP;
static void LoadBMP_UsingMEMDEV(const char *sFilename, int x0, int y0)
{
	/* 进入临界段 */
	taskENTER_CRITICAL();
	/* 打开图片 */
	result = f_open(&file, sFilename, FA_OPEN_EXISTING | FA_READ);
	if ((result != FR_OK))
	{
		printf("文件打开失败！\r\n");
		_acbuffer[0]='\0';
	}
	
	/* 申请一块动态内存空间 */
	hMem = GUI_ALLOC_AllocZero(file.fsize);
	/* 转换动态内存的句柄为指针 */
	_acbuffer = GUI_ALLOC_h2p(hMem);

	/* 读取图片数据到动态内存中 */
	result = f_read(&file, _acbuffer, file.fsize, &f_num);
	if(result != FR_OK)
	{
		printf("文件读取失败！\r\n");
	}
	/* 读取完毕关闭文件 */
	f_close(&file);
	/* 退出临界段 */
	taskEXIT_CRITICAL();
	/* 创建内存设备 */
	hBMP = GUI_MEMDEV_CreateEx(x0, y0,                     /* 起始坐标 */
														 GUI_BMP_GetXSize(_acbuffer),/* x方向尺寸 */
														 GUI_BMP_GetYSize(_acbuffer),/* y方向尺寸 */
														 GUI_MEMDEV_HASTRANS);/* 带透明度的内存设备 */
  /* 选择内存设备 */
  GUI_MEMDEV_Select(hBMP);
  /* 绘制BMP到内存设备中 */
  GUI_BMP_Draw(_acbuffer, x0, y0);
  /* 选择内存设备，0表示选中LCD */
  GUI_MEMDEV_Select(0);
  /* 释放内存 */
  GUI_ALLOC_Free(hMem);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/**
  * @brief GUI主任务
  * @note 无
  * @param 无
  * @retval 无
  */
void MainTask(void)
{
	int i = 0;
	
	GUI_SetFont(GUI_FONT_20B_ASCII);
  GUI_SetTextMode(GUI_TM_XOR);
  
  /* 加载BMP图片数据到内存设备 */
  LoadBMP_UsingMEMDEV("0:/image/ngc7635.bmp",
                      (LCD_GetXSize()-768)/2,
                      (LCD_GetYSize()-432)/2);
  
	while (1)
	{
		i++;
		switch(i)
		{
			case 1:
				t0 = GUI_GetTime();
				/* 直接从外部存储器绘制BMP图片 */
				ShowBMPEx("0:/image/ngc7635.bmp",
                  (LCD_GetXSize()-768)/2,
                  (LCD_GetYSize()-432)/2);
				t1 = GUI_GetTime();
        GUI_DispStringAt("GUI_BMP_DrawEx()", 0, 0);
        printf("\r\n直接从外部存储器绘制BMP：%dms\r\n",t1 - t0);
				break;
			case 2:
				t0 = GUI_GetTime();
				/* 加载BMP图片到内存中并绘制 */
				ShowBMP("0:/image/ngc7635.bmp",
                (LCD_GetXSize()-768)/2,
                (LCD_GetYSize()-432)/2);
				t1 = GUI_GetTime();
        GUI_DispStringAt("GUI_BMP_Draw()", 0, 0);
        printf("加载BMP到内存中并绘制：%dms\r\n",t1 - t0);
				break;
			case 3:
        /* 显示内存设备中的BMP图片 */
        t0 = GUI_GetTime();
        /* 从内存设备写入LCD */
        GUI_MEMDEV_CopyToLCDAt(hBMP,
                               (LCD_GetXSize()-768)/2,
                               (LCD_GetYSize()-432)/2);
        t1 = GUI_GetTime();
        GUI_DispStringAt("USE MEMDEV", 0, 0);
        printf("使用内存设备显示BMP：%dms\r\n",t1 - t0);
				break;
			default:
				i = 0;
				break;
		}
		GUI_Delay(2000);
		GUI_Clear();
	}
}

/*************************** End of file ****************************/
