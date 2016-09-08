#ifndef __SMARTUI_INTERFACE_H__
#define __SMARTUI_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif

	enum layer_enum {
		top_layer = 0,
		bottom_layer,
		invalid_layer,
	};

	enum align_enum {
		center_align = 0,
		left_align,
		invalid_align,
	};

	struct view_struct {
		int top;
		int bottom;
		int left;
		int right;
		unsigned int rgb;
		enum layer_enum layer;
		enum align_enum align;

		char *mem;
		char *str;
		List top_list;
		void (*__display_handler)(struct view_struct *v, char *s, void *info);
		void (*__destory_handler)(void *v);

		bool global;
		bool hide;
		struct view_struct *bottom_view;
	};

	struct textview_struct {
		struct view_struct v;
		unsigned int ascii_font_width; /* 8,16 */
		unsigned int chinese_font_width; /* 16, 24  */
		unsigned int font_height; /* 16,  24 */
		unsigned int font_spacing; /* 1 */
		unsigned int line_spacing; /* 1 */
	};

	struct imageview_struct {
		struct view_struct v;
	};

	struct barview_struct {
		struct view_struct v;
		char *bmp_data;
	};

	/**
	 * @brief set textview's alignment
	 */
	extern void smartui_textview_set_align(struct textview_struct *tv, enum align_enum align);
	/**
	 * @brief set textview's font
	 */
	extern void smartui_textview_font_set(struct textview_struct *tv, int font_height);
	/**
	 * @brief clear a textview
	 */
	extern void smartui_textview_clear(struct textview_struct *tv);
	/**
	 * @brief dispaly a textview
	 */
	extern int smartui_textview_display(struct textview_struct *tv, char *s);
	/**
	 * @brief build a textview
	 */
	extern struct textview_struct *smartui_textview(struct view_struct *v);
	/**
	 * @brief set textview's global flag
	 */
	extern int smartui_textview_set_global(struct textview_struct *tv);
	/**
	 * @brief clear textview's global flag
	 */
	extern int smartui_textview_clear_global(struct textview_struct *tv);
	/**
	 * @brief destory a textview
	 */
	void smartui_textview_destory(void *v);
	/**
	 * @brief clear a imageview
	 */
	extern void smartui_imageview_clear(struct imageview_struct *tv);
	/**
	 * @brief dispaly a imageview
	 */
	extern int smartui_imageview_display(struct imageview_struct *tv, char *s);
	/**
	 * @brief hide a imageview
	 */
	extern int smartui_imageview_hide(struct imageview_struct *iv);
	/**
	 * @brief appear a imageview
	 */
	extern int smartui_imageview_appear(struct imageview_struct *iv);
	/**
	 * @brief set imageview's global flag
	 */
	extern int smartui_imageview_set_global(struct imageview_struct *tv);
	/**
	 * @brief clear imageview's global flag
	 */
	extern int smartui_imageview_clear_global(struct imageview_struct *tv);
	/**
	 * @brief build a imageview
	 */
	extern struct imageview_struct *smartui_imageview(struct view_struct *v);
	/**
	 * @brief destory a imageview
	 */
	extern void smartui_imageview_destory(void *v);
	/**
	 * @brief set barview's pattern
	 */
	extern int smartui_barview_set_pattern(struct barview_struct *bv, char *s, int *token);
	/**
	 * @brief get barview's column number
	 */
	extern int smartui_barview_get_col_num(struct barview_struct *bv);
	/**
	 * @brief clear a barview
	 */
	extern void smartui_barview_clear(struct barview_struct *bv);
	/**
	 * @brief display a barview
	 */
	extern int smartui_barview_display(struct barview_struct *bv, int *height_data);
	/**
	 * @brief destory a barview
	 */
	extern void smartui_barview_destory(void *v);
	/**
	 * @brief build a barview
	 */
	extern struct barview_struct *smartui_barview(struct view_struct *v);
	/**
	 * @brief get screen width
	 */
	extern unsigned int smartui_get_xres(void);
	/**
	 * @brief get screen height
	 */
	extern unsigned int smartui_get_yres(void);
	/**
	 * @brief clear views which is on the v.
	 */
	extern void smartui_clear_top(struct view_struct *v);
	/**
	 * @brief clear screen
	 */
	extern void smartui_clear_screen(void);
	/**
	 * @brief update display
	 */
	extern void smartui_sync(void);
	/**
	 * @brief Startup smart ui.
	 *
	 * @return On success returns 0, return -1 if an error occurred.
	 */
	extern int smartui_startup(void);
	/**
	 * @brief Shutdown smart ui, and release of resources.
	 */
	extern void smartui_shutdown(void);

#ifdef __cplusplus
}
#endif
#endif	/* __SMARTUI_INTERFACE_H__ */
