/******************************************************************************
 * Copyright (c) 2016, 深圳市彩易生活科技有限公司.
 * File        :elife_doss.h
 * Version     : 1.0
 * Description : -智能设备控制sdk
 *
 * --------------------
 * author: huangzhen  2016-7-20 10:24:00
 * --------------------
*******************************************************************************/

#ifndef ELIFE_DOSS_H_
#define ELIFE_DOSS_H_

//错误码定义

//成功
//语音:打开/关闭  XX位置 XX设备  成功
#define _ELIFE_DOSS_OK 0

//登录失败，手机号或者密码不正确
//语音:您还不是彩易智能家具会员，请先注册为彩易能家居会员
#define _ELIFE_DOSS_ERR_LOGIN 5600

//语义三元组校验失败
//语音:您描述的还不是很清晰，请再讲一遍
#define _ELIFE_DOSS_ERR_PARAM 5601

//没找到满足条件的设备
//语音:您描述的还不是很清晰，请再讲一遍
#define _ELIFE_DOSS_ERR_DEVICE 5603

//没有匹配的模式
//语音:您描述的还不是很清晰，请再讲一遍
#define _ELIFE_DOSS_ERR_MODE 5604

//暂不支持此类控制
//语音:暂不支持XX控制
#define _ELIFE_DOSS_ERR_SUPPORT 5605

//无法解析的指令
//语音:我还没听懂，请再讲一遍
#define _ELIFE_DOSS_ERR_CMD 5610

//网络错误
//语音:请检查网络是否正常
#define _ELIFE_DOSS_ERR_NET 5611

//系统错误
//语音:系统错误
#define _ELIFE_DOSS_ERR_SYSTEM 5699


/***************************************************************************
 * Copyright (c) 2016, 深圳市彩易生活科技有限公司.
 * Function name: send_wise_ctl_cmd
 *
 * Description: 将设备的名称、位置、控制指令三元组发送到彩易云执行
 *
 * input Parameters:  char* name -- 设备名
 *                    char* position  -- 设备位置
                      char* cmd  --控制命令

 * output Parameters: char* play_voice --语音播报彩易云的处理结果
 *
 * Return Value:  int
 *                  =0 , 处理成功
 *                  =5600  登录失败，手机号或者密码不正确
 *                  =5601  语义三元组校验失败
 *                  =5603  没找到满足条件的设备
 *                  =5604  没有匹配的模式
 *                  =5605  暂不支持此类控制
 *                  =5610  无法解析的指令
 *                  =5611  网络错误
 *                  =5699  系统错误
***************************************************************************/

typedef struct ai_elife_movie_info_t {
	char *name;
	int sequence;
	char *director;
	char *player;
	char *type;
	char *area;
}ai_elife_movie_info;

typedef struct ai_elife_t{
	char *name;
	char *position;
	char *cmd;
	char *play_voice;
	ai_elife_movie_info movie;
}ai_elife_t;	//*/


extern ai_elife_t ai_elife;
extern int send_wise_ctl_cmd(char* name, char* position, char* cmd, char* play_voice);
extern int send_wise_movie_cmd(char* cmd, char* name, int seq, char* director, char* player,
   		 char* type, char* area, char* play_voice);
extern void ai_elife_init(void);
extern void ai_elife_free(void);
extern int ai_elife_command(void);
extern void ai_elife_command_free(void);
extern int ai_elife_movie(void);
extern void ai_elife_movie_free(void);
#endif
