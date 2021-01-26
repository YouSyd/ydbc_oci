#include "ocidbtools.h"

//ROCIConnect GOCIConn;

pConnPool Gconn_pool=NULL;

double dbconn_percent(pConnPool pool) 
{
    int __unused__=0;
    int __index__=0;
    while(__index__<(_msize(pool)/sizeof(RConnPool))) {
        if((pool->conn+__index__)->status==1) __unused__++;
        __index__++;
    }
    return __unused__*1.0/(_msize(pool)/sizeof(RConnPool));
}

void alter_connect_seconds(pConnPool pool,int wait_sec)
{
    if(pool&&wait_sec>0&&wait_sec!=pool->max_waiting_sec) pool->max_waiting_sec=wait_sec;
}

//��ʼ����ʱ�����ȵ���
pConnPool GetOCIPool()
{
    return Gconn_pool;
}

pConnPool AllocateConnPool(char* svr,char* usr,char* pwd)
{
    pConnPool pool=(pConnPool)calloc(1,sizeof(RConnPool));
    if(pool==NULL) return NULL;
    
    pool->conn=(pOCIConnect)calloc(OCI_CONN_MAXCOUNT,sizeof(ROCIConnect));
    pool->mutex=(HANDLE*)calloc(OCI_CONN_MAXCOUNT,sizeof(HANDLE));
    
    if(pool->conn==NULL||pool->mutex==NULL) return NULL;
        
    for(int index=0;index<OCI_CONN_MAXCOUNT;index++) {
        pool->mutex[index]=CreateMutex(NULL,FALSE,NULL);//������mutex����ǰ�߳�ֱ��ռ��
        if(NULL==pool->mutex[index]) return NULL;
        //ReleaseMutex(pool->mutex[index]);
    }
    
    strcpy(pool->svr,svr);
    strcpy(pool->usr,usr);
    strcpy(pool->pwd,pwd);
    
    pool->status=1;//�Ѿ������ڴ棬����ʹ���ˡ�
    pool->max_waiting_sec=OCI_CONN_MAXWAITTIME;
    Gconn_pool=pool;
    
    //��ʹ��OCI�̺߳�������Ҫ.
    //OCIThreadProcessInit();
    
    return pool;
}

//���������ʱ�����
void ClearConnPool()
{
    pConnPool conn_pool=GetOCIPool();
    if(conn_pool) {
        WaitForMultipleObjects(OCI_CONN_MAXCOUNT,conn_pool->mutex,TRUE,INFINITE);
        for(int index=0;index<OCI_CONN_MAXCOUNT;index++) {
            CloseHandle(conn_pool->mutex[index]);
            ReleaseOCIConnection(conn_pool->conn+index);
        }
        
        free(conn_pool->conn);
        free(conn_pool->mutex);
        
        free(conn_pool);
    }
}

int InitialOCIConnection(pOCIConnect conn,char* server_name,char* user,char* pwd)
{
    sword errcode=0;
    
    if(conn==NULL) {
        printf("debug:function %s, line no:%d",__FUNCTION__,__LINE__);
        return -1;
    }
    if(conn->status==1) {//�Ѿ���ʼ�� 
        printf("0x%08X/%s:connection�Ѿ�����ʼ��-���ӵ�ַ:0x%08X.\n",GetCurrentThreadId(),__FUNCTION__,conn);
        return 0;
    }
    
    conn->env=(OCIEnv*)0;
    conn->err=(OCIError*)0;
    conn->server=(OCIServer*)0;
    conn->svrctx=(OCISvcCtx*)0;
    conn->session=(OCISession*)0;
    conn->retcode=0;
    memset(conn->error_desc,0x00,sizeof(conn->error_desc));
    
    /*
    errcode=OCIEnvCreate((OCIEnv**)&conn->env,
                         (ub4)OCI_DEFAULT,
                         (dvoid*)0,
                         (dvoid * (*)(dvoid *,size_t)) 0,
                         (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
                         (void (*)(dvoid *, dvoid *)) 0, 
                         (size_t)0, 
                         (dvoid **) 0);
    if(errcode!=0) {
        return -1;
    }*/
    
    errcode=OCIEnvNlsCreate((OCIEnv**)&conn->env,
                         (ub4)OCI_THREADED,//OCI_DEFAULT,
                         (dvoid*)0,
                         (dvoid * (*)(dvoid *,size_t)) 0,
                         (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
                         (void (*)(dvoid *, dvoid *)) 0, 
                         (size_t)0, 
                         (dvoid **)0,
                         0,
                         0);
    if(errcode!=OCI_SUCCESS) {
        printf("0x%08X/%s:���������ʼ��ʧ��.\n",GetCurrentThreadId(),__FUNCTION__);
        return -1;
    }
    else {
        printf("0x%08X/%s:���������ʼ���ɹ�.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    //����������
    if(OCI_SUCCESS!=OCIHandleAlloc((dvoid*)conn->env,(dvoid**)&conn->err,OCI_HTYPE_ERROR,0,(dvoid**)0)) {
        printf("0x%08X/%s:��������ʼ��ʧ��.\n",GetCurrentThreadId(),__FUNCTION__);
        return -1;
    }
    else {
        printf("0x%08X/%s:����������ɹ�.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    if(OCI_SUCCESS!=OCIHandleAlloc((dvoid*)conn->env,(dvoid**)&conn->server,OCI_HTYPE_SERVER,0,(dvoid**)0)) {
        printf("0x%08X/%s:��������ʼ��ʧ��.\n",GetCurrentThreadId(),__FUNCTION__);
        return -1;
    }
    else {
        printf("0x%08X/%s:����������ɹ�.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    if(OCI_SUCCESS!=OCIHandleAlloc((dvoid*)conn->env,(dvoid**)&conn->svrctx,OCI_HTYPE_SVCCTX,0,(dvoid**)0)) {
        printf("0x%08X/%s:�����ľ����ʼ��ʧ��.\n",GetCurrentThreadId(),__FUNCTION__);
        return -1;
    }
    else {
        printf("0x%08X/%s:�����ľ������ɹ�.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    if(OCI_SUCCESS!=OCIHandleAlloc((dvoid*)conn->env,(dvoid**)&conn->session,OCI_HTYPE_SESSION,0,(dvoid**)0)) {
        printf("0x%08X/%s:�Ự�����ʼ��ʧ��.\n",GetCurrentThreadId(),__FUNCTION__);
        return -1;
    }
    else {
        printf("0x%08X/%s:�Ự�������ɹ�.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    //�������ӷ�����
    if(OCI_SUCCESS!=OCIServerAttach(conn->server,conn->err,(text*)server_name,strlen(server_name),0)) {
        sb4 errcode=0;
        text errbuf[512];
        
        OCIErrorGet((dvoid*)conn->err,(ub4)1,(text*)NULL,&errcode,errbuf,(ub4)sizeof(errbuf),OCI_HTYPE_ERROR);
        strcpy(conn->error_desc,(char*)errbuf);
        printf("0x%08X/%s:���ݿ����������ʧ��/%s.svr/%s,usr/%s,pwd/%s\n",
               GetCurrentThreadId(),__FUNCTION__,conn->error_desc,server_name,user,pwd);
        return -1;
    }
    else {
        printf("0x%08X/%s:�ɹ���������������.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    //��server context
    if(OCI_SUCCESS!=OCIAttrSet((dvoid *)conn->svrctx,OCI_HTYPE_SVCCTX,(dvoid*)conn->server,(ub4)0,OCI_ATTR_SERVER,conn->err)) {
        printf("0x%08X/%s:�����������ʧ��.\n",GetCurrentThreadId(),__FUNCTION__);
        return -1;
    }
    else {
        printf("0x%08X/%s:�����ĳɹ��󶨷���.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    OCIAttrSet((dvoid*)conn->session,
               (ub4)OCI_HTYPE_SESSION,
               (dvoid*)user,
               (ub4)strlen(user),
               (ub4)OCI_ATTR_USERNAME,
               conn->err);
    OCIAttrSet((dvoid*)conn->session,
               (ub4)OCI_HTYPE_SESSION,
               (dvoid*)pwd,
               (ub4)strlen(pwd),
               (ub4)OCI_ATTR_PASSWORD,
               conn->err);
     
    //SessionӦ�÷���dataset����,����ִ��
    conn->retcode=OCISessionBegin(conn->svrctx,conn->err,conn->session,OCI_CRED_RDBMS,(ub4)OCI_DEFAULT);
    //if(errcode==OCI_ERROR)
    if(conn->retcode!=OCI_SUCCESS&&conn->retcode!=OCI_SUCCESS_WITH_INFO)
    {
        sb4 errcode=0;
        text errbuf[512];
        
        OCIErrorGet((dvoid*)conn->err,(ub4)1,(text*)NULL,&errcode,errbuf,(ub4)sizeof(errbuf),OCI_HTYPE_ERROR);
        strcpy(conn->error_desc,(char*)errbuf);
        printf("0x%08X/%s:��ʼ�Ựʧ��.\n",GetCurrentThreadId(),__FUNCTION__);
        return -1;
    }
    else {
        printf("0x%08X/%s:�Ự�����ɹ�.\n",GetCurrentThreadId(),__FUNCTION__);
    }
    
    //��session�󶨵�server context.
    OCIAttrSet((dvoid*)conn->svrctx,
               (ub4)OCI_HTYPE_SVCCTX,
               (dvoid*)conn->session,
               (ub4)0,
               (ub4)OCI_ATTR_SESSION,
               conn->err);
    
    //�̰߳�ȫ��Ӧ��ûɶ��
    //OCIThreadProcessInit();
    
    printf("0x%08X/%s:���������������.\n",GetCurrentThreadId(),__FUNCTION__);
    
    conn->status=1;
    
    return 0;
}

int OpenOCIDataSet(pOCIDataSet dataset)
{
    sword status=0;

    //���������
    if(dataset->err==(OCIError*)0) {
        OCIHandleAlloc((dvoid*)dataset->connect->env,(dvoid**)&dataset->err,OCI_HTYPE_ERROR,0,(dvoid**)0);    
    }
    
    status=\
    OCIStmtExecute(dataset->connect->svrctx,dataset->stmt,dataset->err,
                   (ub4)0,//���������ȡֵ���ڲ�ѯ��ͷǲ�ѯ�����Ҫ
                          //For non-SELECT statements, the number of times this statement is executed is equal to iters - rowoff.
                          //For SELECT statements, if iters is nonzero, then defines must have been done for the statement handle. The execution fetches iters rows into these predefined buffers and prefetches more rows depending upon the prefetch row count. If you do not know how many rows the SELECT statement will retrieve, set iters to zero.
                   (ub4)0,
                   (CONST OCISnapshot*)NULL,
                   (OCISnapshot*)NULL,
                   OCI_DEFAULT);
    dataset->retcode=status;
    if(status==OCI_STILL_EXECUTING) {
        printf("0x%08X/%s:%s.\n",GetCurrentThreadId(),__FUNCTION__,"OCIStmtExecute��������ִ����");
        return 0;//���������������,�ر���ִ�д�������ɾ�Ĳ���ʱ
    }
    else if(status==OCI_SUCCESS) {
        return 0;
    }
    else {
        GetOCIDataSetError(dataset);
        printf("0x%08X/%s:%s.\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
}

int SetDataSetClause(pOCIDataSet dataset,char* sql_clause)
{
    //������Ϣ
    QueryPerformanceFrequency(&dataset->debug.freq);
    QueryPerformanceCounter(&dataset->debug.begin_tick);
    dataset->debug.exec_time=time(NULL);
    dataset->debug.total_fetch_rows=0;
    dataset->debug.app_mem_alloc_times=0;
    dataset->debug.app_mem_bytes=0;
    
    int sqlclause_length=strlen(sql_clause);    
    
    if(dataset->sql_clause==NULL) {
        dataset->sql_clause=(text*)malloc(sizeof(char)*1024*4);
        memset(dataset->sql_clause,0,_msize(dataset->sql_clause)/sizeof(char));
    }
    
    if(_msize(dataset->sql_clause)/sizeof(char)<=sqlclause_length) {
        dataset->sql_clause=(text*)realloc(dataset->sql_clause,sqlclause_length+1024);
        memset(dataset->sql_clause,0,_msize(dataset->sql_clause)/sizeof(char));
    }
    
    strcpy((char*)dataset->sql_clause,sql_clause);
    dataset->debug.clause=(char*)dataset->sql_clause;
    
    dataset->connect=GetOCIConnection();
    if(dataset->connect==NULL) {
        return -1;
    }
    else {
        dataset->connect->status=2;//ʹ����
        printf("0x%08X:�ɹ�ȡ������-���ӵ�ַ:0x%08X\n",GetCurrentThreadId(),dataset->connect);
    }
    
    dataset->err=(OCIError*)0;
    dataset->retcode=0;
    //���������
    OCIHandleAlloc((dvoid*)dataset->connect->env,(dvoid**)&dataset->err,OCI_HTYPE_ERROR,0,(dvoid**)0);
    
    OCIHandleAlloc((dvoid*)dataset->connect->env,
                   (dvoid **)&dataset->stmt,
                   OCI_HTYPE_STMT,
                   (size_t)0,
                   (dvoid**)0);
    {
        //��ȡ��Ӧ�����ErrCode.
    }
    OCIStmtPrepare(dataset->stmt,
                   dataset->err,
                   dataset->sql_clause,
                   (ub4)strlen((char*)dataset->sql_clause),
                   (ub4)OCI_NTV_SYNTAX,
                   (ub4)OCI_DEFAULT);    
    
    return 0;
}

void display_exec_info(pOCIDebug exec)
{
    printf("Thread ID:0x%08X\n",GetCurrentThreadId());
    printf("SQL clause:\n");
    printf("%s\n",exec->clause);
    printf("fetch rows in total:%d,batch fetch rows settings:%d,total columns:%d\n",
           exec->total_fetch_rows,exec->multi_fetch_rows,exec->column_count);
    tm* st=localtime(&exec->exec_time);
    printf("exec time:%d-%02d-%02d %02d:%02d:%02d ,time costs:%lf\n",
           st->tm_year+1900,
           st->tm_mon+1,
           st->tm_mday,
           st->tm_hour,
           st->tm_min,
           st->tm_sec,
           (double)(exec->end_tick.QuadPart-exec->begin_tick.QuadPart)/(double)exec->freq.QuadPart);
    printf("memory operation:mem alloc %d times,size:%zd bytes\n",exec->app_mem_alloc_times,exec->app_mem_bytes);
}

int CloseOCIDataSet(pOCIDataSet dataset)
{
    if(dataset->stmt) {
        OCIStmtRelease(dataset->stmt,dataset->err,(text*)0,0,OCI_DEFAULT);
        OCIHandleFree((dvoid*)dataset->stmt,OCI_HTYPE_STMT);
    }
    
    //debug
    QueryPerformanceCounter(&dataset->debug.end_tick);
    display_exec_info(&dataset->debug);
    
    if(dataset->sql_clause) free(dataset->sql_clause);
    if(dataset->err!=(OCIError*)0) OCIHandleFree((dvoid*)dataset->err,OCI_HTYPE_ERROR);
    
    //�ͷŶ�conn��ռ��
    if(dataset->connect) {
        if(!ReleaseMutex(dataset->connect->mutex)) printf("0x%08X:mutex release failed,%d\n",GetCurrentThreadId(),GetLastError());;
        dataset->connect->status=1;//����ʹ��
    }
    return 0;
}

void GetOCIDataSetError(pOCIDataSet dataset)
{
    sword errcode=0;
    
    memset(dataset->error_desc,0x00,sizeof(dataset->error_desc)/sizeof(char));
    OCIErrorGet((dvoid*)dataset->err,(ub4)1,(text*)NULL,&errcode,(text*)dataset->error_desc,(ub4)sizeof(dataset->error_desc),OCI_HTYPE_ERROR);
    sprintf(&dataset->error_desc[strlen(dataset->error_desc)],"ErrorCode:%d",errcode);
    if(errcode!=0) {
        FILE* file=NULL;
        if(NULL==(file=fopen("ocidberror.log","a+"))) return;
        fwrite("ERROR:\n",sizeof(char),strlen("ERROR:\n"),file);
        fwrite(dataset->error_desc,sizeof(char),strlen(dataset->error_desc),file);
        fwrite("Clause:\n",sizeof(char),strlen("Clause:\n"),file);
        fwrite((char*)dataset->sql_clause,sizeof(char),strlen((char*)dataset->sql_clause),file);
        fwrite("\n",sizeof(char),strlen("\n"),file);
        fclose(file);
    }
}

//ûд�������ĳ���
//ʵ����Ҫ����˼
DWORD WaitForSuperMultipleObjects(DWORD nCount,const HANDLE* lpHandles,BOOL bWaitAll,DWORD dwMilliseconds)
{
    if(nCount<=MAXIMUM_WAIT_OBJECTS) return WaitForMultipleObjects(nCount,lpHandles,bWaitAll,dwMilliseconds);
    else {
        if(dwMilliseconds==INFINITE) {
            //�е��ѣ���������޵ȴ�������������...
        }
        else {
            if(!bWaitAll) {
                //�⵼�µȴ�ʱ�䷭�˺ܶ౶?
                //����ȡ���ڵȴ��Ķ����Ƿ񷵻أ�����з��أ�û�����⣬����������أ�������
                for(int index=0;index<MAXIMUM_WAIT_OBJECTS;index=+MAXIMUM_WAIT_OBJECTS) {
                    int handle_count=((nCount-index)>=MAXIMUM_WAIT_OBJECTS)?MAXIMUM_WAIT_OBJECTS:(nCount-index);
                    DWORD wait_code=WaitForMultipleObjects(handle_count,lpHandles+index,bWaitAll,dwMilliseconds);
                    
                    if(wait_code-WAIT_OBJECT_0>=0&&wait_code-WAIT_OBJECT_0<handle_count) {
                        return wait_code;
                    }
                    else {
                        //continue.
                    }
                }
            }
            else {
                //��� bWaitAll==TRUE,��ζ��Ҫ�����е�handles����
                for(int index=0;index<MAXIMUM_WAIT_OBJECTS;index=+MAXIMUM_WAIT_OBJECTS) {
                    int handle_count=((nCount-index)>=MAXIMUM_WAIT_OBJECTS)?MAXIMUM_WAIT_OBJECTS:(nCount-index);
                    DWORD wait_code=WaitForMultipleObjects(handle_count,lpHandles+index,bWaitAll,dwMilliseconds);
                    //û��break,����ִ��
                    //���Ƿ���ֵ��ô�죿
                }
            }
        }
        
    }
}

pOCIConnect GetOCIConnection()
{
    pConnPool conn_pool=GetOCIPool();
    if(!conn_pool) return NULL;
            
    //�Ƿ�������⣬������ִ�е�ʱ�򡣱��������߳�ͬʱִ��waitfor... �᲻��һ��mutex���ͷŵ��������̵߳�waitfor �����أ�
    //waitfor...���ں�̬������Ӧ�ò��������⡣
    if(conn_pool->status==0) return NULL;
    
    //������ȴ�ʱ��
    int wait_sec=conn_pool->max_waiting_sec;
    //��������������64�����ᱨ��������Ҫ����һ�£�
    
    DWORD wait_code=WaitForMultipleObjects(OCI_CONN_MAXCOUNT,conn_pool->mutex,FALSE,INFINITE/*1000*wait_sec*/);
    if(wait_code==WAIT_TIMEOUT) {
        printf("0x%08X:��ȡ���ӳ�ʱ(%d/%lf)\n",GetCurrentThreadId(),wait_sec,DBCONN_PERCENT(GetOCIPool()));
        //if(DBCONN_PERCENT(GetOCIPool())>0.5) {
        //if(wait_sec<15) {
            //�����ȴ�ʱ��
            //alter_connect_seconds(conn_pool,wait_sec++);
            //��Ҫ�ݹ飬���ں�̬�����ݹ�ᵼ��cpu���
            //return GetOCIConnection();
        //}
        //else
        //��̫����ִ�� 
        return NULL;//��ʱ��
    }
    else if(wait_code==WAIT_FAILED) {
        printf("0x%08X:��ȡ����ʧ�ܣ��ȴ������������-%d\n",GetCurrentThreadId(),GetLastError());
        return NULL;//������
    }
    else if(wait_code==WAIT_ABANDONED) {
        printf("0x%08X:��ȡ����ʧ�ܣ�������δ��ռ���߳��ͷ�.\n",GetCurrentThreadId());
        return NULL;//���˴������ˣ�ռ��mutex���̵߳�ִ����϶�û���ͷ�mutex.
    }
    else if(wait_code-WAIT_OBJECT_0>=0&&wait_code-WAIT_OBJECT_0<OCI_CONN_MAXCOUNT) {
        int conn_index=wait_code-WAIT_OBJECT_0;
        printf("0x%08X:��ǰ��ȡ��������-%d\n",GetCurrentThreadId(),conn_index);
        (conn_pool->conn+conn_index)->mutex=conn_pool->mutex[conn_index];
        
        //��ȡ����.
        if(0!=InitialOCIConnection(conn_pool->conn+conn_index,conn_pool->svr,conn_pool->usr,conn_pool->pwd)) {
            printf("0x%08X:���ӳ�ʼ��ʧ��.\n",GetCurrentThreadId());
            //ReleaseMutex((conn_pool->conn+conn_index)->mutex);
            return NULL;
        }
        
        if(DBCONN_PERCENT(GetOCIPool())<0.5) {
            //�����ȴ�ʱ��
            alter_connect_seconds(conn_pool,OCI_CONN_MAXWAITTIME);
        }
        return conn_pool->conn+conn_index;
    }
    else {
        printf("0x%08X:��ȡ����ʧ��.\n",GetCurrentThreadId());
        return NULL;
    }
}

int ReleaseOCIConnection(pOCIConnect connect)
{
    OCISessionEnd(connect->svrctx,connect->err,connect->session,(ub4)OCI_DEFAULT); 
    
    if(connect->session) OCIHandleFree((dvoid*)connect->session,OCI_HTYPE_SESSION);
    if(connect->svrctx) OCIHandleFree((dvoid*)connect->svrctx,OCI_HTYPE_SVCCTX);
    if(connect->server) OCIHandleFree((dvoid*)connect->server,OCI_HTYPE_SERVER);
    if(connect->err) OCIHandleFree((dvoid*)connect->err,OCI_HTYPE_ERROR);
    
    if(connect->env) {
        //�����������Env�������ͷ�
        OCIHandleFree((dvoid*)connect->env,OCI_HTYPE_ENV);
        //���ǿ������ϵ�ʹ��OCILogon/OCILogoff �Ժܶ���������������ͷ�
    }
    return 0;
}

#ifdef DEBUG_OCIDBTEST

sb4 cbk_read_lob(void* ctxp,CONST void* bufxp,oraub8 len,ub1 piece,void** changed_buffpp,oraub8* changed_lenp)
{
    static ub4 piece_count = 0; 
    piece_count++; 

    switch (piece)
    {
        case OCI_LAST_PIECE:     /*--- buffer processing code goes here ---*/ 
            printf("callback read the %d th piece\n\n", piece_count);
            piece_count = 0;
        break;
        case OCI_FIRST_PIECE:   /*--- buffer processing code goes here ---*/ 
            printf("callback read the %d th piece\n", piece_count);
            printf("%s\n",(char*)bufxp);
            /* --Optional code to set changed_bufpp and changed_lenp if the
            buffer must be changed dynamically --*/
            break;
        case OCI_NEXT_PIECE:   /*--- buffer processing code goes here ---*/
            printf("callback read the %d th piece\n", piece_count);
            /* --Optional code to set changed_bufpp and changed_lenp if the
            buffer must be changed dynamically --*/
            break;
        default:
            printf("callback read error: unknown piece = %d.\n", piece);
        return OCI_ERROR;
    } 
    return OCI_CONTINUE;
}

void CLobFetch_Test()
{
    ROCIDataSet dataset={0};
    char query_clause[256]="select GetExpString('04') query_str from dual";
    //"select wm_concat('hello word'||chr(13)||' this is a test ') from dual";
    sword retcode=0;
    OCIDefine* def_lob=(OCIDefine*)0;
    ub2 dbtype;
    char buffer[1024*12]="";
    int buffer_size=sizeof(buffer)/sizeof(char);
    
    
    SetDataSetClause(&dataset,query_clause);
    dbtype=SQLT_CLOB;
    //�������Lobָ��ͻ��滻ȡ����. oci�ڴ���clob��Ч�ʴ����ǡ�
    //OCIDefineByPos(dataset.stmt,&def_lob,dataset.err,1,(dvoid*)buffer,(sword)sizeof(buffer),dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    OCILobLocator* lob_plt;
    OCIDescriptorAlloc(dataset.connect->env,(dvoid**)&lob_plt,OCI_DTYPE_LOB,0,NULL);
    OCILobCreateTemporary(dataset.connect->svrctx,dataset.err,lob_plt,OCI_DEFAULT,SQLCS_IMPLICIT,OCI_TEMP_CLOB,FALSE,OCI_DURATION_SESSION);
    //OCILobWrite(dataset.connect->svrctx,dataset.connect->err,lob_plt,(ub4*)&buffer_size,1,buffer,sizeof(buffer), OCI_ONE_PIECE, NULL, NULL, OCI_DEFAULT, SQLCS_IMPLICIT);
    OCIDefineByPos(dataset.stmt,&def_lob,dataset.err,1,&lob_plt,sizeof(OCILobLocator*),dbtype,0,0,0,OCI_DEFAULT);
    /*
    {
        GetOCIDataSetError(&dataset);
        printf("%s\n",dataset.error_desc);
    }
    */
    retcode=OpenOCIDataSet(&dataset);
    while(TRUE) {
        if(retcode==OCI_NO_DATA) {
            break;
        }
        else if(retcode==OCI_SUCCESS||retcode==OCI_SUCCESS_WITH_INFO) {
            ub4 amt=0;
            ub4 offset=1;
            ub4 bufl=sizeof(buffer);
            OCILobRead(dataset.connect->svrctx,
                        dataset.err,
                        lob_plt,
                        (ub4*)&amt,//&buffer_size,
                        offset,
                        (void*)&buffer[0],
                        (ub4)bufl,
                        (void*)0,
                        (OCICallbackLobRead)NULL,
                        (ub2)OCI_DEFAULT,
                        SQLCS_IMPLICIT);
            /*
            OCILobRead2(dataset.connect->svrctx,
                        dataset.err,
                        lob_plt,
                        (oraub8*)&buffer_size,
                        (oraub8*)&buffer_size,
                        1,
                        buffer,
                        sizeof(buffer),
                        (ub1)0,
                        (void*)OCI_FIRST_PIECE,
                        cbk_read_lob,
                        (ub2)0,
                        SQLCS_IMPLICIT);*/
            printf("[%d],[%s]",amt,&buffer[0]);
            //FILE* file=fopen("a.txt","ab+");
            //if(file) fwrite(buffer,sizeof(char),amt,file);
            //if(file) fclose(file);
            
            //memset(buffer,0x00,sizeof(buffer));
            retcode=OCIStmtFetch(dataset.stmt,dataset.err,1,OCI_FETCH_NEXT, OCI_DEFAULT);
        }
        else {
            GetOCIDataSetError(&dataset);
            printf("Error occurred,%d:%s\n",retcode,dataset.error_desc);
            break;
        }        
    }
    
    OCIDescriptorFree(lob_plt,OCI_DTYPE_LOB);
    OCILobClose(dataset.connect->svrctx,dataset.err,lob_plt);    
    CloseOCIDataSet(&dataset);   
}

void BatchFetch_Test()
{
/*
typedef struct _STRUCT_EXP_ {
    char expcode[10];
    char exppath[256];
}RExp,*pExp;
*/

typedef struct _STRUCT_EXP_ {
    char* expcode;//[10];
    char* exppath;//[256];
    OCIRowid* exprowid;
}RExp,*pExp;
    
    ROCIDataSet dataset={0};
    char query_clause[1024]="select distributorcode,senddirectory,rowid from t_exppath order by distributorcode";
    
    //RExp exp[100]={0};
    
    int fetch_rows=1000;
    pExp pex=(pExp)malloc(sizeof(RExp)*fetch_rows);
    if(pex==NULL) return;
    /* not ok
    for(int index=0;index<fetch_rows;index++) {
        (pex+index)->expcode=(char*)calloc(10,sizeof(char));
        (pex+index)->exppath=(char*)calloc(256,sizeof(char));
    }*/
    //OK.
    char* tmp_expcode=(char*)calloc(10*fetch_rows,sizeof(char));
    char* tmp_exppath=(char*)calloc(256*fetch_rows,sizeof(char));
    char* tmp_rowid=(char*)calloc(32*fetch_rows,sizeof(char));
    
    sword retcode=0;
    
    SetDataSetClause(&dataset,query_clause);
    
    for(int index=0;index<fetch_rows;index++) {
        (pex+index)->expcode=tmp_expcode+index*10;
        (pex+index)->exppath=tmp_exppath+index*256;
        OCIDescriptorAlloc(dataset.connect->env,
                           (dvoid**)&(pex+index)->exprowid,
                           (ub4)OCI_DTYPE_ROWID,
                           (size_t)0,
                           (dvoid**)0);
        
    }
    
    OCIDefine* def_expcode=(OCIDefine*)0;
    OCIDefine* def_exppath=(OCIDefine*)0;
    OCIDefine* def_exprowid=(OCIDefine*)0;
    ub2 dbtype;
    
    dbtype=SQLT_STR;
    OCIDefineByPos(dataset.stmt,&def_expcode,dataset.err,1,(dvoid*)tmp_expcode,(sword)9/*sizeof*//*_msize(pex->expcode)/sizeof(char)*/,dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    OCIDefineArrayOfStruct(def_expcode,dataset.err,sizeof(char)*10,0,0,0);   
    dbtype=SQLT_STR;
    OCIDefineByPos(dataset.stmt,&def_exppath,dataset.err,2,(dvoid*)tmp_exppath,(sword)255/*sizeof*//*_msize(pex->exppath)/sizeof(char)*/,dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    OCIDefineArrayOfStruct(def_exppath,dataset.err,sizeof(char)*256,0,0,0);  
    dbtype=SQLT_RDD;
    OCIDefineByPos(dataset.stmt,&def_exprowid,dataset.err,3,(dvoid*)&pex->exprowid,sizeof(OCIRowid*)/*sizeof*//*_msize(pex->exppath)/sizeof(char)*/,dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    OCIDefineArrayOfStruct(def_exprowid,dataset.err,sizeof(RExp),0,0,0);   
    
    retcode=OpenOCIDataSet(&dataset);
    while(TRUE) {
        if(retcode==OCI_NO_DATA) {
            //��ȡ���һ�ε�����
            int lastfetch_rows=0;
            
            OCIAttrGet(dataset.stmt,OCI_HTYPE_STMT,&lastfetch_rows,(ub4*)NULL,OCI_ATTR_ROWS_FETCHED,dataset.err);
            {
                for(int index=0;index<lastfetch_rows;index++) {
                    char rowid_buff[256]="";
                    int rowid_len=256;
               
                    OCIRowidToChar((OCIRowid*)(pex+index)->exprowid,
                                   (text*)rowid_buff,
                                   (ub2*)&rowid_len,
                                   dataset.err);
                    printf("%s,%s,%s\n",(pex+index)->expcode,(pex+index)->exppath,rowid_buff);
                }
            }
            break;
        }
        else if(retcode==OCI_SUCCESS||retcode==OCI_SUCCESS_WITH_INFO) {
            {
                int c_fetch_rows=0;
                OCIAttrGet(dataset.stmt,OCI_HTYPE_STMT,&c_fetch_rows,(ub4*)NULL,OCI_ATTR_ROWS_FETCHED,dataset.err);
                for(int index=0;index<c_fetch_rows;index++) {
                    char rowid_buff[256]="";
                    int rowid_len=256;
               
                    OCIRowidToChar((OCIRowid*)(pex+index)->exprowid,
                                   (text*)rowid_buff,
                                   (ub2*)&rowid_len,
                                   dataset.err);
                    printf("%s,%s,%s\n",(pex+index)->expcode,(pex+index)->exppath,rowid_buff);
                }
            }
            retcode=OCIStmtFetch2(dataset.stmt,dataset.err,fetch_rows,OCI_FETCH_NEXT,1,OCI_DEFAULT);
        }
        else {
            GetOCIDataSetError(&dataset);
            printf("%s\n",dataset.error_desc);
            break;
        }        
    }
    
    //2020-1-23
    if(pex) free(pex);
    if(tmp_expcode) free(tmp_expcode);
    if(tmp_exppath) free(tmp_exppath);
    
    CloseOCIDataSet(&dataset);    
}

void BatchFetchCLob_Test()
{
typedef struct _STRUCT_CLOB_ {
    int id;
    char* name;
    OCILobLocator* clob;
}RCLob,*pCLob;
    
    ROCIDataSet dataset={0};
    char query_clause[1024]="select query_id,query_name,query_clause from tquerys";
    int fetch_rows=1000;
    pCLob pcl=(pCLob)malloc(sizeof(RCLob)*fetch_rows);
    if(pcl==NULL) return;
    char* tmp_name=(char*)calloc(256*fetch_rows,sizeof(char));
    
    sword retcode=0;
    
    SetDataSetClause(&dataset,query_clause);
    
    for(int index=0;index<fetch_rows;index++) {
        (pcl+index)->name=tmp_name+index*256;
        
        //CLOB�Ĳ�����Щ���⣬����Ҫ���������������÷�����ʱ�ռ䣬����װ��.
        OCIDescriptorAlloc(dataset.connect->env,
                           (dvoid**)&(pcl+index)->clob,
                           (ub4)OCI_DTYPE_LOB,
                           (size_t)0,
                           (dvoid**)0);
        //�����ȴ�����ʱ�ռ�
        OCILobCreateTemporary(dataset.connect->svrctx,
                              dataset.err,
                              (pcl+index)->clob,
                              OCI_DEFAULT,
                              SQLCS_IMPLICIT,
                              OCI_TEMP_CLOB,
                              FALSE,
                              OCI_DURATION_SESSION);
    }
    
    OCIDefine* def_id=(OCIDefine*)0;
    OCIDefine* def_name=(OCIDefine*)0;
    OCIDefine* def_clob=(OCIDefine*)0;
    ub2 dbtype;
    
    dbtype=SQLT_VNU;
    OCIDefineByPos(dataset.stmt,&def_id,dataset.err,1,(dvoid*)&pcl->id,(sword)sizeof(OCINumber)/*sizeof*//*_msize(pex->expcode)/sizeof(char)*/,dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    OCIDefineArrayOfStruct(def_id,dataset.err,sizeof(RCLob),0,0,0);   
    dbtype=SQLT_STR;
    OCIDefineByPos(dataset.stmt,&def_name,dataset.err,2,(dvoid*)pcl->name,(sword)256/*sizeof*//*_msize(pex->exppath)/sizeof(char)*/,dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    OCIDefineArrayOfStruct(def_name,dataset.err,sizeof(char)*256,0,0,0);  
    dbtype=SQLT_CLOB;
    OCIDefineByPos(dataset.stmt,&def_clob,dataset.err,3,(dvoid*)&pcl->clob,sizeof(OCILobLocator*)/*sizeof*//*_msize(pex->exppath)/sizeof(char)*/,dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    OCIDefineArrayOfStruct(def_clob,dataset.err,sizeof(RCLob),0,0,0);   
    
    retcode=OpenOCIDataSet(&dataset);
    while(TRUE) {
        if(retcode==OCI_NO_DATA) {
            //��ȡ���һ�ε�����
            int lastfetch_rows=0;
            
            OCIAttrGet(dataset.stmt,OCI_HTYPE_STMT,&lastfetch_rows,(ub4*)NULL,OCI_ATTR_ROWS_FETCHED,dataset.err);
            {
                for(int index=0;index<lastfetch_rows;index++) {
                    //clob ȡ��
                    char* tmp_buffer=NULL;
                    
                    ub4 amt=0;
                    ub4 offset=1;
                    ub4 bufl=0;//1024*64;
                
                    //�����⣬ȡ�����ַ������������ֽڸ���
                    OCILobGetLength(dataset.connect->svrctx,
                                    dataset.err,
                                    (pcl+index)->clob,
                                    &bufl);
                    //˫�ֽ��ַ����ֵ
                    tmp_buffer=(char*)calloc(bufl*2+1,sizeof(char));//����.
                              
                    OCILobRead(dataset.connect->svrctx,
                            dataset.err,
                            (pcl+index)->clob,
                            (ub4*)&amt,//&buffer_size,
                            offset,
                            (void*)tmp_buffer,
                            (ub4)bufl*2,
                            (void*)0,
                            (OCICallbackLobRead)NULL,
                            (ub2)OCI_DEFAULT,
                            SQLCS_IMPLICIT);
                    printf("[%s],[%s]\n",(pcl+index)->name,tmp_buffer);
                    
                    free(tmp_buffer);
                }
            }
            break;
        }
        else if(retcode==OCI_SUCCESS||retcode==OCI_SUCCESS_WITH_INFO) {
            {
                int c_fetch_rows=0;
                OCIAttrGet(dataset.stmt,OCI_HTYPE_STMT,&c_fetch_rows,(ub4*)NULL,OCI_ATTR_ROWS_FETCHED,dataset.err);
                for(int index=0;index<c_fetch_rows;index++) {
                    //clob ȡ��
                    char* tmp_buffer=NULL;
                    
                    ub4 amt=0;
                    ub4 offset=1;
                    ub4 bufl=0;//1024*64;
                
                    //�����⣬ȡ�����ַ������������ֽڸ���
                    OCILobGetLength(dataset.connect->svrctx,
                                    dataset.err,
                                    (pcl+index)->clob,
                                    &bufl);
                    //˫�ֽ��ַ����ֵ
                    tmp_buffer=(char*)calloc(bufl*2+1,sizeof(char));//����.
                    
                    OCILobRead(dataset.connect->svrctx,
                            dataset.err,
                            (pcl+index)->clob,
                            (ub4*)&amt,//&buffer_size,
                            offset,
                            (void*)tmp_buffer,
                            (ub4)bufl*2,
                            (void*)0,
                            (OCICallbackLobRead)NULL,
                            (ub2)OCI_DEFAULT,
                            SQLCS_IMPLICIT);
                    printf("[%s],[%s]\n",(pcl+index)->name,tmp_buffer);
                    
                    free(tmp_buffer);
                }
            }
            //memset(exp,0x00,sizeof(exp));
            retcode=OCIStmtFetch2(dataset.stmt,dataset.err,fetch_rows,OCI_FETCH_NEXT,1,OCI_DEFAULT);
        }
        else {
            GetOCIDataSetError(&dataset);
            printf("%s\n",dataset.error_desc);
            break;
        }        
    }
    
    //2020-1-23
    if(pcl) free(pcl);
    if(tmp_name) free(tmp_name);
    
    CloseOCIDataSet(&dataset);    
}


void SingleFetch_Test()
{
    ROCIDataSet oci_dataset={0};
    char query_clause[1024]= "select distributorcode,senddirectory from t_exppath order by distributorcode";
    char exp_code[9]="";
    char exp_path[256]="";
    sword retcode=0;
    
    SetDataSetClause(&oci_dataset,query_clause);//OCIStmtPrepare
    //��������󶨱���
    //Prepare->OCIDefineByPos->OCIStmtExecute...
    ub2 dbtype;
    
    retcode=OpenOCIDataSet(&oci_dataset);//OCIStmtExecute
    {
        //���Ի�ȡ����Ϣ
        OCIParam *param;
        ub4 index=1;
        ub2 type,type_length;
        text* column_name;
        ub4 col_name_len;
        int col_count=0;
        
        OCIAttrGet(oci_dataset.stmt,OCI_HTYPE_STMT,(void*)&col_count,(ub4*)0,(ub4)OCI_ATTR_PARAM_COUNT,oci_dataset.err);
        printf("���ƣ�%d��\n",col_count);       
        while(OCI_SUCCESS==OCIParamGet(oci_dataset.stmt,OCI_HTYPE_STMT,oci_dataset.err,(void**)&param,index)) {
            OCIAttrGet((void*)param,
                       (ub4)OCI_DTYPE_PARAM,
                       (void**)&column_name,
                       (ub4*)&col_name_len,
                       (ub4)OCI_ATTR_NAME,
                       oci_dataset.err);
                       
            OCIAttrGet((void*)param,
                       (ub4)OCI_DTYPE_PARAM,
                       (void*)&type,
                       (ub4*)0,
                       (ub4)OCI_ATTR_DATA_TYPE,
                       oci_dataset.err);
                       
            OCIAttrGet((void*)param,
                       (ub4)OCI_DTYPE_PARAM,
                       (void*)&type_length,
                       (ub4*)0,
                       (ub4)OCI_ATTR_DATA_SIZE,
                       oci_dataset.err);
            
            //OCI_ATTR_PRECISION
            //OCI_ATTR_SCALE
            //OCI_ATTR_IS_NULL
            printf("%s:(%s,%d)\n",column_name,(type==OCI_TYPECODE_VARCHAR2?"�ַ���":"other"),type_length);
            index++;
        }
    }
    OCIDefine* pdef=(OCIDefine*)0;
    dbtype=SQLT_STR;
    OCIDefineByPos(oci_dataset.stmt,&pdef,oci_dataset.err,1,(dvoid*)exp_code,(sword)sizeof(exp_code),dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    dbtype=SQLT_STR;
    OCIDefineByPos(oci_dataset.stmt,&pdef,oci_dataset.err,2,(dvoid*)exp_path,(sword)sizeof(exp_path),dbtype,(dvoid*)0,(ub2*)0,(ub2*)0,OCI_DEFAULT);
    
    while(TRUE) {
        if(retcode==OCI_NO_DATA) {
            break;
        }
        else if(retcode==OCI_SUCCESS||retcode==OCI_SUCCESS_WITH_INFO) {
            printf("%s,%s\n",exp_code,exp_path);
            
            memset(exp_code,0x00,sizeof(exp_code));
            memset(exp_path,0x00,sizeof(exp_path));
            retcode=OCIStmtFetch(oci_dataset.stmt,oci_dataset.err,1,OCI_FETCH_NEXT, OCI_DEFAULT);
        }
        else {
            GetOCIDataSetError(&oci_dataset);
            printf("%s\n",oci_dataset.error_desc);
            break;
        }        
    }
    
    CloseOCIDataSet(&oci_dataset);    
}

void main()
{
    ROCIDataSet oci_dataset={0};
    char query_clause[1024]="select distributorcode,senddirectory from t_exppath order by distributorcode";
    char exp_code[9]="";
    char exp_path[256]="";
    sword retcode=0;
    
    AllocateConnPool("","test","test");
    
    SingleFetch_Test();
    BatchFetch_Test();
    BatchFetchCLob_Test();
    CLobFetch_Test();
    
    ClearConnPool();
}
#endif