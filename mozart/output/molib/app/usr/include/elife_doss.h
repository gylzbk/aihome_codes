/******************************************************************************
 * Copyright (c) 2016, �����в�������Ƽ����޹�˾.
 * File        :elife_doss.h
 * Version     : 1.0
 * Description : -�����豸����sdk
 * 
 * --------------------
 * author: huangzhen  2016-7-20 10:24:00
 * --------------------
*******************************************************************************/

#ifndef ELIFE_DOSS_H_
#define ELIFE_DOSS_H_

//�����붨��

//�ɹ� 
//����:��/�ر�  XXλ�� XX�豸  �ɹ�
#define _ELIFE_DOSS_OK 0  

//��¼ʧ�ܣ��ֻ��Ż������벻��ȷ
//����:�������ǲ������ܼҾ߻�Ա������ע��Ϊ�����ܼҾӻ�Ա
#define _ELIFE_DOSS_ERR_LOGIN 5600  

//������Ԫ��У��ʧ��,�޴����豸
//����:�������Ļ����Ǻ����������ٽ�һ��
#define _ELIFE_DOSS_ERR_DEVICE 5601

//������Ԫ��У��ʧ�ܣ��޴���λ��
//����:�������Ļ����Ǻ����������ٽ�һ��
#define _ELIFE_DOSS_ERR_POSITION 5602

//û�ҵ������������豸,�������������豸
//����:�������Ļ����Ǻ����������ٽ�һ��
#define _ELIFE_DOSS_ERR_CONDITION 5603  

//û��ƥ���ģʽ
//����:�������Ļ����Ǻ����������ٽ�һ��
#define _ELIFE_DOSS_ERR_MODE 5604  

//�ݲ�֧�ִ������
//����:�ݲ�֧��XX����
#define _ELIFE_DOSS_ERR_SUPPORT 5605  

//�޷�������ָ��
//����:�һ�û���������ٽ�һ��
#define _ELIFE_DOSS_ERR_CMD 5610

//�������
//����:���������Ƿ�����
#define _ELIFE_DOSS_ERR_NET 5611

//ϵͳ����
//����:ϵͳ����
#define _ELIFE_DOSS_ERR_SYSTEM 5699

/***************************************************************************
 * Copyright (c) 2016, �����в�������Ƽ����޹�˾.
 * Function name: send_wise_ctl_cmd
 *
 * Description: ���豸�����ơ�λ�á�����ָ����Ԫ�鷢�͵�������ִ��
 *
 * input Parameters:  char* name -- �豸��
 *                    char* position  -- �豸λ��
                      char* cmd  --��������
                      
 * output Parameters: char* play_voice --�������������ƵĴ�����
 *
 * Return Value:  int
 *                  =0 , ����ɹ�
 *                  =5600  ��¼ʧ�ܣ��ֻ��Ż������벻��ȷ
 *                  =5601  ������Ԫ��У��ʧ��               
 *                  =5603  û�ҵ������������豸
 *                  =5604  û��ƥ���ģʽ
 *                  =5605  �ݲ�֧�ִ������
 *                  =5610  �޷�������ָ��
 *                  =5611  �������
 *                  =5699  ϵͳ����
***************************************************************************/
int send_wise_ctl_cmd(char* name, char* position, char* cmd, char* play_voice);


/***************************************************************************
 * Copyright (c) 2016, �����в�������Ƽ����޹�˾.
 * Function name: send_wise_movie_cmd
 *
 * Description: ����Ӱ�����ơ����ݡ����ݡ����͡���������Ϣ���͵�������ִ��
 *
 * input Parameters:  char* cmd --����:���š���ͣ���˳���
                      char* name -- ��Ӱ/���Ӿ���
                      int   seq  -- ���Ӿ缯��
                      char* director  -- ����
                      char* player  --��Ա
                      char* type  --����
                      char* area  --����
                      
 * output Parameters: char* play_voice --�������������ƵĴ�����
 *
 * Return Value:  int
 *                  =0 , ����ɹ�
 *                  =5600  ��¼ʧ�ܣ��ֻ��Ż������벻��ȷ
 *                  =5806  û�ҵ����뿴�ĵ�Ӱ  
 *                  =5611  �������
 *                  =5699  ϵͳ����
***************************************************************************/
int send_wise_movie_cmd(char* cmd, char* name, int seq, char* director, char* player,
    char* type, char* area, char* play_voice);

/***************************************************************************
 * Copyright (c) 2016, �����в�������Ƽ����޹�˾.
 * Function name: send_wise_voice_cmd
 *
 * Description: ����������͵�������ִ��
 *
 * input Parameters:  char* voice_cmd -- ����ָ�json����ʽ
                      
 * output Parameters: char* play_voice --�������������ƵĴ�����
 *
 * Return Value:  int
 *                  =0 , ����ɹ�
 *                  =5600  ��¼ʧ�ܣ��ֻ��Ż������벻��ȷ
 *                  =5601  ������Ԫ��У��ʧ��               
 *                  =5603  û�ҵ������������豸
 *                  =5604  û��ƥ���ģʽ
 *                  =5605  �ݲ�֧�ִ������
 *                  =5610  �޷�������ָ��
 *                  =5611  �������
 *                  =5699  ϵͳ����
 *                  =5806  û�ҵ����뿴�ĵ�Ӱ  
***************************************************************************/
int send_wise_voice_cmd(char* voice_cmd, char* play_voice);

int get_info(char* user_info);


#endif
