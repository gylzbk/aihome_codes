#ifndef __SHARE_MEMORY_HEAD__
#define __SHARE_MEMORY_HEAD__

#ifdef  __cplusplus
extern "C" {
#endif
	/**
	 * @brief status of the domain.
	 */
	typedef enum {
		/**
		 * @brief wait for event response done.
		 */
		WAIT_RESPONSE,
		/**
		 * @brief event response done.
		 */
		RESPONSE_DONE,
		RESPONSE_PAUSE,
		RESPONSE_CANCEL,
	} module_status;

	/**
	 * @brief The index for the shared memory operations.
	 */
	typedef enum {
		/**
		 * @brief unknown domain
		 */
		UNKNOWN_DOMAIN,
		/**
		 * @brief dlna-render domain
		 */
		RENDER_DOMAIN,
		/**
		 * @brief airplay domain
		 */
		AIRPLAY_DOMAIN,
		/**
		 * @brief atalk domain
		 */
		ATALK_DOMAIN,
		/**
		 * @brief localplayer domain
		 */
		LOCALPLAYER_DOMAIN,
		/**
		 * @brief vr domain
		 */
		VR_DOMAIN,
		/**
		 * @brief bt phone domain
		 */
		BT_HS_DOMAIN,
		/**
		 * @brief bt music domain
		 */
		BT_AVK_DOMAIN,
		/**
		 * @brief lapsule domain
		 */
		LAPSULE_DOMAIN,
		/**
		 * @brief tone domain
		 */
		TONE_DOMAIN,
		/**
		 * @brief custom domain
		 */
		CUSTOM_DOMAIN,
	} memory_domain;

	/**
	 * @brief By enumeration value obtain the corresponding enumeration string
	 * Generally, it used only for debug.
	 */
	extern char *module_status_str[];

	/**
	 * @brief how many status now.
	 * Generally, it used only for debug.
	 */
	extern int get_status_cnt(void);

	/**
	 * @brief According to the index number of shared memory, to get the string of corresponding domain
	 * Generally, it used only for debug.
	 */
	extern char *memory_domain_str[];

	/**
	 * @brief how many domains now.
	 * Generally, it used only for debug.
	 */
	extern int get_domain_cnt(void);

	/**
	 * @brief  Shared memory initialization, before operating the shared memory
	 * and must be called once.
	 *
	 * @return On success returns 0, return -1 if an error occurred.
	 */
	extern int share_mem_init(void);

	/**
	 * @brief  Empty shared memory, shared memory for all domains are set to zero.
	 *
	 * @return On success returns 0, return -1 if an error occurred.
	 */
	extern int share_mem_clear(void);

	/**
	 * @brief  Destruction of shared memory, after each call share_mem_init,
	 * when no longer in use sharememory API, call the function.
	 *
	 * @return On success returns 0, return -1 if an error occurred.
	 */
	extern int share_mem_destory(void);

	/**
	 * @brief  Gets the value of the specified index field.
	 *
	 * @param domain [in]	The specified index
	 * @param status [out]	State value
	 *
	 * @return On success returns 0, return -1 if an error occurred.
	 */
	extern int share_mem_get(memory_domain domain, module_status *status);

	/**
	 * @brief  Set the value of the specified index field.
	 *
	 * @param domain [in]	The specified index
	 * @param status [in]	State value
	 *
	 * @return On success returns 0, return -1 if an error occurred.
	 */
	extern int share_mem_set(memory_domain domain, module_status status);
#ifdef  __cplusplus
}
#endif
#endif /* __SHARE_MEMORY_HEAD__ */
