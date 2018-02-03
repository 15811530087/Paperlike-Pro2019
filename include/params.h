#ifndef __PARAMS_H__
#define __PARAMS_H__

//#define DEBUG

#ifdef DEBUG
#define DSPRINT	printf
#else
#define DSPRINT(X,...) {}
#endif

#define WND_TITLE	"PaperLikeHD"
#define WINDOW_VBOX_PADDING	0

#define ERROR_THRESH_COUNT	5

typedef unsigned char bool;
enum {
	false = 0,
	true = 1
};

//#pragma pack(1)
typedef struct _SignalData
{
    struct _SignalDataTypeHead
    {
        unsigned char uSignalDataType : 4;
#define DS_PACKET_TYPE_BEAT					1
#define DS_PACKET_TYPE_SCREENSAVER			2
#define DS_PACKET_TYPE_TURNOFFMONITOR		3
#define DS_PACKET_TYPE_MONITORRESOLUTION	4
#define DS_PACKET_TYPE_THRESHOLD			5
#define DS_PACKET_TYPE_CLEANMONITOR			6
#define DS_PACKET_TYPE_MODE					7
#define DS_PACKET_TYPE_THRESHOLD_FLOYD      8
#define DS_PACKET_TYPE_THRESHOLD_A2         9
#define DS_PACKET_TYPE_THRESHOLD_LOW_A2     10
#define DS_PACKET_TYPE_THRESHOLD_LOW_A5     11
#define DS_PACKET_TYPE_ADVANCED_SPEED       12
        unsigned char Reserved : 4;
    }SignalDataTypeHead;
    
    union _SignalDataType
    {
	unsigned char value;
        // 1.心跳信号（发送频率降低-1min） -> DS_PACKET_TYPE_BEAT
        struct _Beat
        {
            // 第1个bit: 1:正常;0:异常
            // 第2个bit: 0:正常;1:异常
            unsigned char uBeatSign : 1;
            unsigned char uBeatSignOff : 1;
            unsigned char Reserved : 6;
        }Beat;
        
        // 2.屏保计数器  ->  DS_PACKET_TYPE_SCREENSAVER
        struct _ScreenSaver
        {
            // 第1个bit: 1:屏保;0:非屏保
            unsigned char ucScreenSaver : 1;
            unsigned char Reserved : 7;
        }ScreenSaver;
        
        // 3. 关闭显示器  ->  DS_PACKET_TYPE_TURNOFFMONITOR
        struct _TurnOffMonitor
        {
            // 第1个bit: 1:关闭;0:不关闭
            unsigned char ucTurnOffMonitor : 1;
            unsigned char Reserved : 7;
        }TurnOffMonitor;
        
        // 4.分辨率选择  ->  DS_PACKET_TYPE_MONITORRESOLUTION
        struct _MonitorResolution
        {
            // 1~4bit:
            // 其他分辨率:0000
            // 800*600  :0001
            // 1024*768 :0011
            // 1100*825 :0101
            // 1400*1050:0111
            // 1600*1200:1001
            // 1920*1200:1011
            unsigned char ucMonitorResolution : 4;
            unsigned char Reserved : 4;
        }MonitorResolution;
        
        // 5.阈值设置  ->  DS_PACKET_TYPE_THRESHOLD
        struct _Threshold
        {
            unsigned char uThreshold;
            // 1~8bit: 00000000~11111111
        }Threshold;
        
        // 6.清残影  ->  DS_PACKET_TYPE_CLEANMONITOR
        struct _CleanMonitor
        {
            // 第1bit: 触发软件刷新:1;不触发软件刷新:0
            // 第2bit: 触发硬件刷新:1;不触发硬件刷新:0
            unsigned char uCleanMonitorSoft : 1;
            unsigned char uCleanMonitorHard : 1;
            unsigned char Reserved : 6;
        }CleanMonitor;
        
        // 7.显示模式选择  ->  DS_PACKET_TYPE_MODE
        struct _Mode
        {
	#if 0
            // 1~2bit: A2:00;A5:01;16:10;floyd:11
            // 3~4bit: A2:00;A5:01;16:10;floyd:11
            //mode1 normal
            unsigned char DisplayMode1 : 2;
            //centralization
            unsigned char DisplayMode2 : 2;
            //strengthen mode 1
            unsigned char DisplayMode3 : 2;
            //strengthen mode 2
            unsigned char DisplayMode4 : 2;
	#endif
            //mode normal
            unsigned char DisplayMode   : 4;
            //centralization
            unsigned char Reserved      : 4;
        }Mode;
    }SignalDataType;
    
}__attribute__((packed)) SignalData;
//#pragma pop()

#define DS_MONITOR_TRY_COUNT    5

#define DS_REQUEST_FRESH        1
#define DS_REQUEST_MODE_SWITCH  2
#define DS_REQUEST_POWERON      3
#define DS_REQUEST_POWEROFF     4

#define DS_RES_HIGH             1
#define DS_RES_LOW              2

#define DS_RES_800_X_600        2
#define DS_RES_OTHER            1

#define LANG_ZH			1
#define LANG_EN			2

#define DS_MODE_THRESHOLD_LOW       130
#define DS_MODE_THRESHOLD_MEDIUM    170
#define DS_MODE_THRESHOLD_HIGH      190

#define DS_MODE_DEF_THRESHOLD_A5    160
#define DS_MODE_DEF_THRESHOLD_A2    190
#define DS_MODE_DEF_THRESHOLD_FLOYD 140

#define DS_DISPLAY_NORMAL       1
#define DS_DISPLAY_CENTRAL      2
#define DS_DISPLAY_STRENTHEN_1  3
#define DS_DISPLAY_STRENTHEN_2  4

#define DS_MODE_FLOYD           1
#define DS_MODE_A2              2
#define DS_MODE_A16             3
#define DS_MODE_LOW_A5          4
#define DS_MODE_LOW_A2          5
#define DS_MODE_LOW_A61         6

#define DS_FRESH_TYPE_SOFT      1
#define DS_FRESH_TYPE_HAND      2
#define DS_FRESH_TYPE_HARDWARE  3

#define DS_SCREEN_SAVE_TIME     50

#define CH

#ifdef CH
#define CURRENTMODETITLE        "当前模式"
#define SETTINGTITLE            "设置"
#define RESCHOOSETITLE          "分辨率选择"
#define HELPTITLE               "帮助"
#define HIDETITLE               "隐藏面板"
#define EXITTITLE               "退出程序"

#define DS_DISPLAY_MODE_NORMAL      "正常模式"
#define DS_DISPLAY_MODE_CENTRAL     "居中模式"
#define DS_DISPLAY_MODE_STRENTHEN_1 "全屏拉伸模式"
#define DS_DISPLAY_MODE_STRENTHEN_2 "等比例拉伸模式"

#define DS_FRESH_STRING_MODE_SOFT       "软件自动刷新"
#define DS_FRESH_STRING_MODE_HAND       "快捷键手动刷新"
#define DS_FRESH_STRING_MODE_HARDWARE   "硬件刷新"

#else
#define CURRENTMODETITLE        "Current Mode"
#define SETTINGTITLE            "Setting"
#define RESCHOOSETITLE          "Resolution Select"
#define HELPTITLE               "Help"
#define HIDETITLE               "Hide UI"
#define EXITTITLE               "Exit UI"
#endif


/* Main UI string definition */
#define MAIN_UI_A2_EN		"A2"
#define MAIN_UI_A2_CH		"A2"
#define MAIN_UI_A5_EN		"A5"
#define MAIN_UI_A5_CH		"A5"
#define MAIN_UI_A16_EN		"A16"
#define MAIN_UI_A16_CH		"A16"
#define MAIN_UI_SET_EN		"Settings"
#define MAIN_UI_SET_CH		"设置"
#define MAIN_UI_HIDE_EN		"Hide window"
#define MAIN_UI_HIDE_CH		"隐藏"
#define MAIN_UI_HELP_EN		"Help"
#define MAIN_UI_HELP_CH		"帮助"
#define MAIN_UI_EXIT_EN		"Exit"
#define MAIN_UI_EXIT_CH		"退出"
#define MAIN_UI_RES_HIGH_CH	"可变分辨率模式"
#define MAIN_UI_RES_LOW_CH	"固定1100x825模式"
#define MAIN_UI_RES_HIGH_EN	"Flexiable RES mode"
#define MAIN_UI_RES_LOW_EN	"Fix 1100x825 mode"

#define HELP_UI_UPDATE_CH	"检查更新"
#define HELP_UI_UPDATE_EN	"Checking updating"
#define HELP_UI_ABOUT_CH	"关于"
#define HELP_UI_ABOUT_EN	"About"

#define SET_NOR_DISP_CH		"正常显示设置:"
#define SET_NOR_DISP_EN		"Normal display setting:"
#define SET_CUR_MODE_CH		"当前显示模式:"
#define SET_CUR_MODE_EN		"Current show mode:"
#define SET_CONSTRAIT_CH	"对比度调整:"
#define SET_CONSTRAIT_EN	"Constrait adjust:"
#define SET_LOW_CH		"浅"
#define SET_LOW_EN		"low"
#define SET_MEDIUM_CH		"适中"
#define SET_MEDIUM_EN		"medium"
#define SET_DARK_CH		"深"
#define SET_DARK_EN		"dark"
#define SET_SELFDEF_CH		"自定义"
#define SET_SELFDEF_EN		"user def"
#define SET_AUTO_REFRESH_CH	"定时自动刷新"
#define SET_AUTO_REFRESH_EN	"Auto refresh"
#define SET_AUTO_REF_INT_CH	"  自动刷新间隔: "
#define SET_AUTO_REF_INT_EN	"  Refresh frequency: "
#define SET_SOFT_HAND_CH	"按键手动软刷新(Alt+C)"
#define SET_SOFT_HAND_EN	"Hot key soft refresh(Alt+C)"
#define SET_HARD_HAND_CH	"快捷键刷新(Alt+H)"
#define SET_HARD_HAND_EN	"Hot key refresh(Alt+H)"
#define SET_ADV_S_L_EN		"Advanced Speed Setting:"
#define SET_ADV_S_L_CH		"高级加速设置:"
#define SET_ADV_S_N_EN		"Note: The setting of the electronic ink suspension parameters of Paperlike HD, for speediness or darkness adjusting"
#define SET_ADV_S_N_CH		"注: Paperlike HD可进行电子墨滴悬浮参数设置,在一定程度上加速或加黑"

#endif
