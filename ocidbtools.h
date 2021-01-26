#include<Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "oci.h"

#pragma once
#pragma comment(lib,"oci.lib")

typedef struct __debug_info__ {
    int level;//日志等级
    int multi_fetch_rows;//批量抓取行数设置
    int dump_method;//0-dump到屏幕，1-到文件，2-到数据库
    char dump_filename[256];//dump文件名
    
    char* clause; //原始语句
    
    //time costs;
    int total_fetch_rows; //返回总行数
    int column_count; //查询列数量
    size_t app_mem_bytes;//自分配部分
    int app_mem_alloc_times;//自分配次数
    
    LARGE_INTEGER freq;
    LARGE_INTEGER begin_tick;//开始执行tick
    LARGE_INTEGER end_tick;//结束执行tick
    
    time_t exec_time;//开始执行大致时间
    int exec_result;//0-成功 1-失败
    char* exec_msg;//返回消息描述
}ROCIDebug,*pOCIDebug;

typedef struct _STRUCT_OCICONN_ {
    OCIEnv* env;
    OCIError* err;
    OCIServer* server;
    OCISvcCtx* svrctx;//事务是在OCISvcCtx上执行的。那怎么将事务与连接断开，与数据集操作绑定呢？
    OCISession* session;//session 放在哪里？ 放在dataset还是 connect？sesson应该与connect绑定，事务与dataset放在一起
    
    int retcode;
    char error_desc[1024];
    HANDLE mutex;
    int status;//0-尚未初始化，1-已经初始化，2-使用中
}ROCIConnect,*pOCIConnect;

//extern ROCIConnect GOCIConn;

//连接数，暂时没想到写出更加完美的waitfor。。。
#define OCI_CONN_MAXCOUNT MAXIMUM_WAIT_OBJECTS
//建立连接等待时长
#define OCI_CONN_MAXWAITTIME 5

typedef struct __OCI_CONN_POOL__ {
    pOCIConnect conn;
    HANDLE* mutex;
    char svr[256];
    char usr[256];
    char pwd[256];
    int status; //0-初始化，1-内存已分配
    
    //等待时间
    int max_waiting_sec;
    //设想：如果检测到有大量连接请求因超时失败，20%
    //检测当前连接占用情况，如果全部被占用，将调整等待策略，延长最大等待时长
    //如果当前连接使用率不足50%，将缩短等待时长。
    //这样做有什么意义？如果简单粗暴的设置等待时长为无限长呢？
    //程序可能出错，陷入卡死状态。
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
