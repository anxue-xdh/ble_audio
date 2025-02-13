#include "gui.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "user_bsp.h"

#include "bsp_ili9341_lcd.h"
#include "fonts.h"

#include "my_list.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#if (Define_DEBUG == 1)
#define Gui_Debug_Print(...) Uart1_SendData(__VA_ARGS__)
#else
#define Gui_Debug_Print(...)
#endif

#define GUI_SetColor(text, back) LCD_SetColors(text, back)
#define GUI_Show_MusicList(line, str) ILI9341_DispStringLine_EN(LINE(line + 3), str)
#define GUI_Clear(line) ILI9341_DispStringLine_EN(LINE(line + 3), "                          ")
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TaskHandle_t GUI_Task_Handle;

signed char MusicList_Pointer = 0;
char MusicList_Num = 0;
char MusicList[SDFile_Name_Num][SDFile_Name_Len];

/**
typedef struct node
{ // 节点结构
    void *data;     //文件名
    struct node *next;
}yNode;
**/
yList MicList;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void
MusicList_Update(void);
static void MusicList_MovePointer(u8 value);
/* USER CODE END PFP */

/* extern function prototypes -----------------------------------------------*/
/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

void GUI_Init(void)
{
    ILI9341_GramScan(0);                                                  // 显示方向
    ILI9341_Init();                                                       // 初始化LCD
    LCD_SetFont(&Font8x16);                                               // 设置字体
    LCD_SetColors(BLUE, WHITE);                                           // 设置字体颜色和背景色
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);                      // 清屏
    ILI9341_DispStringLine_EN(LINE(1), "        BleAudio_Demo         "); // 显示文字

    // 初始化音乐列表
    list_init(&MicList);
}

void GUI_Task(void *argument)
{
    u32 xReturn = 0;
    while (1)
    {
        // if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
        //     Uart1_SendData("[ERROR] Core obliterated Data!!");
        if (!xTaskNotifyWait(0, 0xFFFF, &xReturn, portMAX_DELAY))
            continue;

        switch (xReturn)
        {
        case GUI_TaskBit_MusicList_Update:
            MusicList_Update();
            break;
        case GUI_TaskBit_Key_Up:
        case GUI_TaskBit_Key_Down:
            MusicList_MovePointer(xReturn);
            break;

        default:
            break;
        }
    }
}

void MusicList_MovePointer(u8 value)
{
    GUI_Show_MusicList(MusicList_Pointer, MusicList[MusicList_Pointer]);

    if (value == GUI_TaskBit_Key_Up)
        MusicList_Pointer--;
    else if (value == GUI_TaskBit_Key_Down)
        MusicList_Pointer++;

    if (MusicList_Pointer >= MusicList_Num)
        MusicList_Pointer = 0;
    else if (MusicList_Pointer < 0)
        MusicList_Pointer = MusicList_Num - 1;

    GUI_SetColor(RED, BLUE);
    GUI_Show_MusicList(MusicList_Pointer, MusicList[MusicList_Pointer]);

    GUI_SetColor(BLUE, WHITE);
}

void MusicList_Update(void)
{
    for (int i = 0; MusicList[i][0] != 0; i++)
    {
        GUI_Clear(i);
        GUI_Show_MusicList(i, MusicList[i]);

        MusicList_Num++;
        // vTaskDelay(10);
    }

    // memset(MusicList, 0, sizeof(MusicList)); // 临时，应该变化方法，保留下列表信息
}

FRESULT Mic_Update(const char *path, char (*list)[64])
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;
    TCHAR name[64];

    nfile = ndir = 0;
    fno.lfname = name;
    fno.lfsize = 64;

    res = f_opendir(&dir, path); /* Open the directory */
    if (res != FR_OK)
    {
        Gui_Debug_Print("Failed to open \"%s\". (%u)\n", path, res);
        return res;
    }

    for (;;)
    {
        res = f_readdir(&dir, &fno); /* Read a directory item */
        if (res != FR_OK || fno.fname[0] == 0)
            break; /* Error or end of dir */
        if (fno.fattrib & AM_DIR)
        { /* Directory */
            Gui_Debug_Print("   <DIR>   %s\n", fno.lfname);
            ndir++;
        }
        else
        { /* File */
            Gui_Debug_Print("%10u %s\n", fno.fsize, fno.lfname);

            if (fno.lfname[0] == 0)
                continue;
            memcpy(list[nfile], fno.lfname, strlen(fno.lfname));

            nfile++;
        }
    }
    f_closedir(&dir);
    Gui_Debug_Print("%d dirs, %d files.\n", ndir, nfile);

    // xTaskNotifyGive(GUI_Task_Handle);
    xTaskNotify(GUI_Task_Handle, GUI_TaskBit_MusicList_Update, eSetBits);
    return res;
}
