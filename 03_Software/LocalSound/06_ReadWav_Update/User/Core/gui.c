#include "gui.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "user_bsp.h"

#include "bsp_ili9341_lcd.h"
#include "fonts.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#if (Define_DEBUG == 1)
#define Gui_Debug_Print(...) Uart1_SendData(__VA_ARGS__)
#else
#define Gui_Debug_Print(...)
#endif

#define GUI_SetColor(text, back) LCD_SetColors(text, back)
#define GUI_Show_MusicList(line, str) ILI9341_DispStringLine_EN(LINE(line + 2), str)
#define GUI_Clear(line) ILI9341_DispStringLine_EN(LINE(line + 2), "                          ")
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TaskHandle_t GUI_Task_Handle;

// 当前GUI中高亮显示的音乐索引
u8 MicList_Idx_Gui = 1;
// 当前正在播放的音乐索引
u8 MicList_Idx_au = 1;
//  char MusicList[SDFile_Name_Num][SDFile_Name_Len];

/**
typedef struct node
{ // 节点结构
    void *data;     //文件名
    struct node *next;
}yNode;
**/
yList MicList;         // 音乐文件列表
yList MicList_Playing; // 播放列表
yList MicList_None; //备用列表
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

static void MicList_destroy_Func(void *data);
static void MicList_ShowAll_Func(void *data, int idx);
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
        xTaskNotifyWait(0, 0xFFFF, &xReturn, portMAX_DELAY);

        switch (xReturn)
        {
        case GUI_TaskBit_MusicList_Update:
            list_traverse(&MicList, MicList_ShowAll_Func);
            // MusicList_Update();
            break;
        case GUI_TaskBit_Key_Up:
        case GUI_TaskBit_Key_Down:
            if (MicList.head == NULL)
            {
                Gui_Debug_Print("MicList is NULL\r\n");
                break;
            }
            MusicList_MovePointer(xReturn);
            break;

        default:
            break;
        }
    }
}

void MusicList_MovePointer(u8 value)
{
    if (MicList.len == 0)
    {
        Gui_Debug_Print("MicList is NULL\r\n");
        return;
    }

    char *misName = (char *)list_get_element(&MicList, MicList_Idx_Gui);
    GUI_Show_MusicList(MicList_Idx_Gui, misName);

    if (value == GUI_TaskBit_Key_Up)
        MicList_Idx_Gui--;
    else if (value == GUI_TaskBit_Key_Down)
        MicList_Idx_Gui++;

    if (MicList_Idx_Gui > MicList.len)
        MicList_Idx_Gui = 1;
    else if (MicList_Idx_Gui <= 0)
        MicList_Idx_Gui = MicList.len;

    GUI_SetColor(RED, BLUE);
    misName = (char *)list_get_element(&MicList, MicList_Idx_Gui);
    GUI_Show_MusicList(MicList_Idx_Gui, misName);

    GUI_SetColor(BLUE, WHITE);
}
void MicList_reinit(void)
{
    list_destroy(&MicList, MicList_destroy_Func);
    if (MicList.head == NULL)
    {
        Gui_Debug_Print("MicList head is NULL\r\n");
    }
    if (MicList.tail == NULL)
    {
        Gui_Debug_Print("MicList tail is NULL\r\n");
    }
    Gui_Debug_Print("MicList len is %d\r\n", MicList.len);
}

void MicList_ShowAll_Func(void *data, int idx)
{
    char *name = (char *)data;
    GUI_Clear(idx);
    GUI_Show_MusicList(idx, name);
}

void MicList_destroy_Func(void *data)
{
    free(data);
}

FRESULT MicList_Update(const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;
    TCHAR name[64];

    nfile = ndir = 0;
    fno.lfname = name;
    fno.lfsize = 64;
    char *tmp = NULL;

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

            {
                int len = strlen(fno.lfname);
                // 直接使用calloc分配内存，自动初始化对应的内存
                tmp = (char *)calloc(len + 1, sizeof(char));
                memcpy(tmp, fno.lfname, len);
                // Gui_Debug_Print("%s\n", tmp);
                // 将数据插入列表
                list_insert(&MicList, tmp);
            }

            nfile++;
        }
    }
    f_closedir(&dir);
    Gui_Debug_Print("%d dirs, %d files.\n", ndir, nfile);

    // xTaskNotifyGive(GUI_Task_Handle);
    xTaskNotify(GUI_Task_Handle, GUI_TaskBit_MusicList_Update, eSetBits);
    return res;
}
