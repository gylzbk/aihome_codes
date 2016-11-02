#ifndef _OTA_DEBUG_
#define _OTA_DEBUG_

/*
 * printf打印出不同颜色
 *    printf("\033[背景色;字颜色m字符串\033[0m");
 * 比如
 *    printf("\033[41;33m黑底黄字\n\033[0m");
 *    printf("\033[41;33m黑底黄字\n33[0m");
 *    printf("\033[31m红字不改背景色\n\033[0m");
 *    printf("\033[41m红背景色不改字颜色\n\033[0m");
 *
 * 颜色代码:
 * 背景色范围: 40--47                   字颜色: 30--37
 *             40: 黑                           30: 黑
 *             41: 红                           31: 红
 *             42: 绿                           32: 绿
 *             43: 黄                           33: 黄
 *             44: 蓝                           34: 蓝
 *             45: 紫                           35: 紫
 *             46: 深绿                         36: 深绿
 *             47: 白色                         37: 白色
 *
 * ANSI控制码:
 *   \033[0m   关闭所有属性
 *   \033[1m   设置高亮度
 *   \03[4m    下划线
 *   \033[5m   闪烁
 *   \033[7m   反显
 *   \033[8m   消隐
 *   \033[30m -- \033[37m   设置前景色
 *   \033[40m -- \033[47m   设置背景色
 *   \033[nA   光标上移n行
 *   \03[nB    光标下移n行
 *   \033[nC   光标右移n行
 *   \033[nD   光标左移n行
 *   \033[y;xH 设置光标位置
 *   \033[2J   清屏
 *   \033[K    清除从光标到行尾的内容
 *   \033[s    保存光标位置
 *   \033[u    恢复光标位置
 *   \033[?25l 隐藏光标
 *   \33[?25h  显示光标
 */

#define COLOR_DEBUG "\033[40;37m"
#define COLOR_ERR "\033[41;37m"
#define COLOR_WARN "\033[43;30m"
#define COLOR_INFO "\033[44;37m"
#define COLOR_CLOSE "\033[0m"

#define OTA_DEBUG 0

#if OTA_DEBUG
#define pr_debug(fmt, args...) \
	do { \
		printf(COLOR_DEBUG"%s:%s:%d [Debug]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)
#else
#define pr_debug(fmt, args...)
#endif

#define pr_info(fmt, args...) \
	do { \
		printf(COLOR_INFO"%s:%s:%d [Info]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)
#define pr_warn(fmt, args...) \
	do { \
		printf(COLOR_WARN"%s:%s:%d [Warn]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)
#define pr_err(fmt, args...) \
	do { \
		printf(COLOR_ERR"%s:%s:%d [Error]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)

#endif // _OTA_DEBUG_
