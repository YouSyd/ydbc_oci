#include<Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "oci.h"

#pragma once
#pragma comment(lib,"oci.lib")

typedef struct __debug_info__ {
    int level;//��־�ȼ�
    int multi_fetch_rows;//����ץȡ��������
    int dump_method;//0-dump����Ļ��1-���ļ���2-�����ݿ�
    char dump_filename[256];//dump�ļ���
    
    char* clause; //ԭʼ���
    
    //time costs;
    int total_fetch_rows; //����������
    int column_count; //��ѯ������
    size_t app_mem_bytes;//�Է��䲿��
    int app_mem_alloc_times;//�Է������
    
    LARGE_INTEGER freq;
    LARGE_INTEGER begin_tick;//��ʼִ��tick
    LARGE_INTEGER end_tick;//����ִ��tick
    
    time_t exec_time;//��ʼִ�д���ʱ��
    int exec_result;//0-�ɹ� 1-ʧ��
    char* exec_msg;//������Ϣ����
}ROCIDebug,*pOCIDebug;

typedef struct _STRUCT_OCICONN_ {
    OCIEnv* env;
    OCIError* err;
    OCIServer* server;
    OCISvcCtx* svrctx;//��������OCISvcCtx��ִ�еġ�����ô�����������ӶϿ��������ݼ��������أ�
    OCISession* session;//session ������� ����dataset���� connect��sessonӦ����connect�󶨣�������dataset����һ��
    
    int retcode;
    char error_desc[1024];
    HANDLE mutex;
    int status;//0-��δ��ʼ����1-�Ѿ���ʼ����2-ʹ����
}ROCIConnect,*pOCIConnect;

//extern ROCIConnect GOCIConn;

//����������ʱû�뵽д������������waitfor������
#define OCI_CONN_MAXCOUNT MAXIMUM_WAIT_OBJECTS
//�������ӵȴ�ʱ��
#define OCI_CONN_MAXWAITTIME 5

typedef struct __OCI_CONN_POOL__ {
    pOCIConnect conn;
    HANDLE* mutex;
    char svr[256];
    char usr[256];
    char pwd[256];
    int status; //0-��ʼ����1-�ڴ��ѷ���
    
    //�ȴ�ʱ��
    int max_waiting_sec;
    //���룺�����⵽�д�������������ʱʧ�ܣ�20%
    //��⵱ǰ����ռ����������ȫ����ռ�ã��������ȴ����ԣ��ӳ����ȴ�ʱ��
    //�����ǰ����ʹ���ʲ���50%�������̵ȴ�ʱ����
    //��������ʲô���壿����򵥴ֱ������õȴ�ʱ��Ϊ���޳��أ�
    //������ܳ������뿨��״̬��
}ROCICONNPOOL,RConnPool,*pConnPool;
extern pConnPool Gconn_pool;

typedef struct _STRUCT_OCIDATASET_ {
    pOCIConnect connect;
    
    int retcode;
    OCIError* err;
    OCIStmt* stmt;
    text* sql_clause;
    char error_desc[1024];
    
    ROCIDebug debug;
}ROCIDataSet,*pOCIDataSet;

pConnPool AllocateConnPool(char* svr,char* usr,char* pwd);
void ClearConnPool();
double dbconn_percent(pConnPool pool);

int InitialOCIConnection(pOCIConnect conn,char* server_name,char* user,char* pwd);
pOCIConnect GetOCIConnection();
int ReleaseOCIConnection(pOCIConnect connect);

int SetDataSetClause(pOCIDataSet dataset,char* sql_clause);
int OpenOCIDataSet(pOCIDataSet dataset);
int CloseOCIDataSet(pOCIDataSet dataset);
void GetOCIDataSetError(pOCIDataSet dataset);

#define DBCONN_PERCENT(pool) dbconn_percent(pool)
