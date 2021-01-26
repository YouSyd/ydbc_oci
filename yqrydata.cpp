#include "yqrydata.h"
#include <process.h>

#pragma comment(lib,"ocidbtools.lib")

int type_test()
{
    RDataItem item={0};
    
    printf("size of RDataItem:%zd\n",sizeof(item));
    
    return 0;
}

int SetDataSetItem(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_pos)
{
    int col_index=0;
    OCIDefine* pdef=(OCIDefine*)0;
    sb2 default_value=0;
    
    if(!dataset||!pcol||!pdata) return -1;
    
    pcol->default_value=0;
    switch((pcol)->type) {
    case SQLT_RDD:
    {
        //rowid.
        if(OCIDescriptorAlloc(dataset->connect->env,
                           (dvoid**)&pdata->var.var_prowid,
                           (ub4)OCI_DTYPE_ROWID,
                           (size_t)0,
                           (dvoid**)0)!=OCI_SUCCESS) return -1;
                           
        if(OCIDefineByPos(dataset->stmt,
                       &pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&pdata->var.var_prowid,
                       (sword)(pcol)->size,
                       SQLT_RDD,
                       (dvoid*)(((pcol)->isnull_flag!=0)?&(pcol->default_value):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) return -1;
    } break;
    case SQLT_DAT:
    {
        //SQLT_DAT : date
        if(OCIDefineByPos(dataset->stmt,
                       &pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&pdata->var.var_date,
                       (sword)(pcol)->size,
                       SQLT_ODT,
                       (dvoid*)(((pcol)->isnull_flag!=0)?&(pcol->default_value):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) return -1;
    } break;
    case SQLT_TIMESTAMP_TZ:
    {
        if(OCIDescriptorAlloc(dataset->connect->env,
                           (dvoid**)&pdata->var.var_ptimestamp,
                           OCI_DTYPE_TIMESTAMP_TZ,
                           (size_t)0,
                           (dvoid**)0)!=OCI_SUCCESS) return -1;
                           
        if(OCIDefineByPos(dataset->stmt,
                       &pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&pdata->var.var_ptimestamp,
                       (sword)(pcol)->size,
                       (pcol)->type,
                       (dvoid*)(((pcol)->isnull_flag!=0)?&(pcol->default_value):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) return -1;
    } break;
    case SQLT_TIMESTAMP:
    {
        //SQLT_TIMESTAMP : timestamp
        if(OCIDescriptorAlloc(dataset->connect->env,
                           (dvoid**)&pdata->var.var_ptimestamp,
                           OCI_DTYPE_TIMESTAMP,
                           (size_t)0,
                           (dvoid**)0)!=OCI_SUCCESS) return -1;
                           
        if(OCIDefineByPos(dataset->stmt,
                       &pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&pdata->var.var_ptimestamp,
                       (sword)(pcol)->size,
                       (pcol)->type,
                       (dvoid*)(((pcol)->isnull_flag!=0)?&(pcol->default_value):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) return -1;
    } break;
    case SQLT_AFC://SQLT_AFC-asci char: char(n)
    case SQLT_CHR:
        (pdata)->var.var_plt=malloc(sizeof(char)*((pcol)->size+1));
        if(!(pdata)->var.var_plt) {
            printf("var_plt malloc failed.\n");
            return -1;
        }
        memset((pdata)->var.var_plt,0x00,_msize((pdata)->var.var_plt)/sizeof(char));
        dataset->debug.app_mem_alloc_times++;
        dataset->debug.app_mem_bytes+=pcol->size+1;
        
        if(OCIDefineByPos(dataset->stmt,
                       &pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)(pdata)->var.var_plt,
                       (sword)(pcol)->size,
                       (pcol)->type,
                       (dvoid*)(((pcol)->isnull_flag!=0)?&(pcol->default_value):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) return -1;
        break;
    case SQLT_NUM:
    {
        if(OCIDefineByPos(dataset->stmt,
                       &pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&(pdata)->var.var_num,
                       (sword)(pcol)->size,
                       SQLT_VNU,
                       (dvoid*)(((pcol)->isnull_flag!=0)?&(pcol->default_value):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) return -1;
    } break;
    case SQLT_CLOB:
    {
        if(OCIDescriptorAlloc(dataset->connect->env,
                           (dvoid**)&pdata->var.var_pclob,
                           OCI_DTYPE_LOB,
                           0,
                           NULL)!=OCI_SUCCESS) return -1;
        if(OCILobCreateTemporary(dataset->connect->svrctx,
                              dataset->err,
                              pdata->var.var_pclob,
                              OCI_DEFAULT,
                              SQLCS_IMPLICIT,
                              OCI_TEMP_CLOB,
                              FALSE,
                              OCI_DURATION_SESSION)!=OCI_SUCCESS) return -1;
                           
        if(OCIDefineByPos(dataset->stmt,
                       &pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&pdata->var.var_pclob,
                       (sword)(pcol)->size,
                       (pcol)->type,
                       (dvoid*)(((pcol)->isnull_flag!=0)?&(pcol->default_value):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) return -1;
    } break;
    default:
        printf("OCI data type:%d,required modification.\n",
                (pcol)->type);
    }

    return 0;
}

int SetDataSetItem_MultiRows(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_pos,int fetch_rows,int column_count)
{
    int col_index=0;
    sb2 default_value=0;
    
    if(!dataset||!pcol||!pdata) return -1;
    
    pcol->default_value=0;
    pcol->pdef=(OCIDefine*)0;
    switch((pcol)->type) {
    case SQLT_RDD:
    {
        //rowid.
        for(int index=0;index<fetch_rows;index++)
        {
            //为每一行分配标识符
            if(OCIDescriptorAlloc(dataset->connect->env,
                               (dvoid**)&(pdata+index*column_count)->var.var_prowid,
                               (ub4)OCI_DTYPE_ROWID,
                               (size_t)0,
                               (dvoid**)0)!=OCI_SUCCESS) {
                return -1;
            }
        }
                           
        if(OCIDefineByPos(dataset->stmt,
                       &pcol->pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&pdata->var.var_prowid,
                       (sword)(pcol)->size,
                       SQLT_RDD,
                       (dvoid*)(((pcol)->isnull_flag!=0)?pcol->default_list:0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) {
            return -1;
        }
        //指定是数组绑定
        if(OCIDefineArrayOfStruct(pcol->pdef,dataset->err,sizeof(RODITEM)*column_count,(((pcol)->isnull_flag!=0)?sizeof(short):0),0,0)!=OCI_SUCCESS) 
            return -1;
    } break;
    case SQLT_DAT:
    {
        //SQLT_DAT : date
        if(OCIDefineByPos(dataset->stmt,
                       &pcol->pdef,
                       dataset->err,
                       col_pos,
                       (dvoid*)&pdata->var.var_date,
                       (sword)(pcol)->size,
                       SQLT_ODT,
                       (dvoid*)(((pcol)->isnull_flag!=0)?(pcol->default_list):0),
                       (ub2*)0,
                       (ub2*)0,
                       OCI_DEFAULT)!=OCI_SUCCESS) {
            return -1;
        }
        //指定是数组绑定
        if(OCIDefineArrayOfStruct(pcol->pdef,dataset->err,sizeof(RODITEM)*column_count,(((pcol)->isnull_flag!=0)?sizeof(short):0),0,0)!=OCI_SUCCESS) {
            return -1;
        }
    } break;
    case SQLT_TIMESTAMP_TZ:
    {
        for(int index=0;index<fetch_rows;index++) {
            if(OCIDescriptorAlloc(dataset->connect->env,
                                  (dvoid**)&(pdata+index*column_count)->var.var_ptimestamp,
                                  OCI_DTYPE_TIMESTAMP_TZ,
                                  (size_t)0,
                                  (dvoid**)0)!=OCI_SUCCESS) {
                return -1;
            }
        }
                           
        if(OCIDefineByPos(dataset->stmt,
                         &pcol->pdef,
                         dataset->err,
                         col_pos,
                         (dvoid*)&pdata->var.var_ptimestamp,
                         (sword)(pcol)->size,
                         (pcol)->type,
                         (dvoid*)(((pcol)->isnull_flag!=0)?(pcol->default_list):0),
                         (ub2*)0,
                         (ub2*)0,
                         OCI_DEFAULT)) {
            return -1;
        }
        
        //指定是数组绑定
        OCIDefineArrayOfStruct(pcol->pdef,dataset->err,sizeof(RODITEM)*column_count,(((pcol)->isnull_flag!=0)?sizeof(short):0),0,0);
    } break;
    case SQLT_TIMESTAMP:
    {
        //SQLT_TIMESTAMP : timestamp
        for(int index=0;index<fetch_rows;index++) {
            if(OCIDescriptorAlloc(dataset->connect->env,
                                  (dvoid**)&(pdata+index*column_count)->var.var_ptimestamp,
                                  OCI_DTYPE_TIMESTAMP,
                                  (size_t)0,
                                  (dvoid**)0)!=OCI_SUCCESS) {
                return -1;
            }
        }
                           
        if(OCIDefineByPos(dataset->stmt,
                          &pcol->pdef,
                          dataset->err,
                          col_pos,
                          (dvoid*)&pdata->var.var_ptimestamp,
                          (sword)(pcol)->size,
                          (pcol)->type,
                          (dvoid*)(((pcol)->isnull_flag!=0)?(pcol->default_list):0),
                          (ub2*)0,
                          (ub2*)0,
                          OCI_DEFAULT)!=OCI_SUCCESS) {
            return -1;
        }
        //指定是数组绑定
        if(OCIDefineArrayOfStruct(pcol->pdef,dataset->err,sizeof(RODITEM)*column_count,(((pcol)->isnull_flag!=0)?sizeof(short):0),0,0)!=OCI_SUCCESS) {
            return -1;
        }
    } break;
    case SQLT_AFC://SQLT_AFC-asci char: char(n)
    case SQLT_CHR:
    {
        pcol->data_mem=calloc(((pcol)->size+1)*fetch_rows,sizeof(char));  
        if(pcol->data_mem==NULL) {
            printf("Multi-Rows模式下设置绑定变量，内存分配失败\n");
            return -1;
        }
        dataset->debug.app_mem_alloc_times++;
        dataset->debug.app_mem_bytes+=(pcol->size+1)*fetch_rows;
        
        //挂上内存指针
        for(int index=0;index<fetch_rows;index++) {
            (pdata+index*column_count)->var.var_plt=(char*)pcol->data_mem+(pcol->size+1)*index;
        }
        
        if(OCIDefineByPos(dataset->stmt,
                          &pcol->pdef,
                          dataset->err,
                          col_pos,
                          (dvoid*)(pdata)->var.var_plt,
                          (sword)(pcol)->size,
                          (pcol)->type,
                          (dvoid*)(((pcol)->isnull_flag!=0)?(pcol->default_list):0),
                          (ub2*)0,
                          (ub2*)0,
                          OCI_DEFAULT)!=OCI_SUCCESS) {
            return -1;
        }
        //指定是数组绑定
        if(OCIDefineArrayOfStruct(pcol->pdef,dataset->err,sizeof(char)*(pcol->size+1),(((pcol)->isnull_flag!=0)?sizeof(short):0),0,0)!=OCI_SUCCESS) {
            return -1;
        }
    } break;
    case SQLT_NUM:
    {
        if(OCIDefineByPos(dataset->stmt,
                          &pcol->pdef,
                          dataset->err,
                          col_pos,
                          (dvoid*)&(pdata)->var.var_num,
                          (sword)(pcol)->size,
                          SQLT_VNU,
                          (dvoid*)(((pcol)->isnull_flag!=0)?(pcol->default_list):0),
                          (ub2*)0,
                          (ub2*)0,
                          OCI_DEFAULT)!=OCI_SUCCESS) {
            return -1;
        }
        //指定是数组绑定
        if(OCIDefineArrayOfStruct(pcol->pdef,dataset->err,sizeof(RODITEM)*column_count,(((pcol)->isnull_flag!=0)?sizeof(short):0),0,0)!=OCI_SUCCESS) {
            return -1;
        }
    } break;
    case SQLT_CLOB:
    {
        for(int index=0;index<fetch_rows;index++) {
            if(OCIDescriptorAlloc(dataset->connect->env,
                                  (dvoid**)&(pdata+index*column_count)->var.var_pclob,
                                  OCI_DTYPE_LOB,
                                  0,
                                  NULL)!=OCI_SUCCESS) {
                return -1;
            }
            if(OCILobCreateTemporary(dataset->connect->svrctx,
                                     dataset->err,
                                     (pdata+index*column_count)->var.var_pclob,
                                     OCI_DEFAULT,
                                     SQLCS_IMPLICIT,
                                     OCI_TEMP_CLOB,
                                     FALSE,
                                     OCI_DURATION_SESSION)!=OCI_SUCCESS) {
                return -1;
            }
        }
        
        if(OCIDefineByPos(dataset->stmt,
                          &pcol->pdef,
                          dataset->err,
                          col_pos,
                          (dvoid*)&pdata->var.var_pclob,
                          (sword)(pcol)->size,
                          (pcol)->type,
                          (dvoid*)(((pcol)->isnull_flag!=0)?(pcol->default_list):0),
                          (ub2*)0,
                          (ub2*)0,
                          OCI_DEFAULT)!=OCI_SUCCESS) {
            return -1;
        }
        //指定是数组绑定
        if(OCIDefineArrayOfStruct(pcol->pdef,dataset->err,sizeof(RODITEM)*column_count,(((pcol)->isnull_flag!=0)?sizeof(short):0),0,0)!=OCI_SUCCESS) {
            return -1;
        }
    } break;
    default: {
        printf("OCI data type:%d,required modification.\n",
                (pcol)->type);
        return -1;
    }
    }
    
    return 0;
}

int SetDataSetColumn(pOCIDataSet dataset,OCIParam*param,pColItem pcol)
{
    text* pcol_name=NULL;
    int col_name_len=256;
    int type_len=sizeof((pcol)->type);
    int isnull_len=sizeof((pcol)->isnull_flag);
    int size_len=sizeof((pcol)->size);
    
    if(!dataset||!pcol||!param) return -1;
        
    if(OCIAttrGet((void*)param,
                  (ub4)OCI_DTYPE_PARAM,
                  (void**)&pcol_name,
                  (ub4*)&col_name_len,
                  (ub4)OCI_ATTR_NAME,
                  dataset->err)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
    memset(pcol->col_name,0x00,sizeof(pcol->col_name));
    strncpy((char*)pcol->col_name,(char*)pcol_name,col_name_len);
     
    if(OCIAttrGet((void*)param,
                  (ub4)OCI_DTYPE_PARAM,
                  (void*)&(pcol)->type,
                  (ub4*)&type_len,
                  (ub4)OCI_ATTR_DATA_TYPE,
                  dataset->err)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
    
    if(OCIAttrGet((void*)param,
               (ub4)OCI_DTYPE_PARAM,
               (void*)&(pcol)->isnull_flag,
               (ub4*)&isnull_len,
               (ub4)OCI_ATTR_IS_NULL,
               dataset->err)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
               
    (pcol)->size=(pcol)->scale=(pcol)->precison=0;
    
    if(OCIAttrGet((void*)param,
                  (ub4)OCI_DTYPE_PARAM,
                  (void*)&(pcol)->size,
                  (ub4*)&size_len,
                  (ub4)OCI_ATTR_DATA_SIZE,
                  dataset->err)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
               
    if(OCIAttrGet((void*)param,
                  (ub4)OCI_DTYPE_PARAM,
                  (void*)&(pcol)->scale,
                  (ub4*)&size_len,
                  (ub4)OCI_ATTR_SCALE,
                  dataset->err)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
               
    if(OCIAttrGet((void*)param,
                  (ub4)OCI_DTYPE_PARAM,
                  (void*)&(pcol)->precison,
                  (ub4*)&size_len,
                  (ub4)OCI_ATTR_PRECISION,
                  dataset->err)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
    
    /*
    printf("%s %d-%d(%d,%d) %d\n",pcol_name,//(pcol+col_index-1)->col_name,
                            (pcol)->type,
                            (pcol)->size,
                            (pcol)->precison,
                            (pcol)->scale,
                            (pcol)->isnull_flag);
    */
    return 0;
}

int SetDataSetInfo(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count)
{
    int col_index=0;
    OCIParam*param=(OCIParam*)0;
    
    if(!dataset||!pcol||!pdata) return -1;
    
    while(col_index<col_count) {
        if(OCI_SUCCESS!=OCIParamGet(dataset->stmt,
                                 OCI_HTYPE_STMT,
                                 dataset->err,
                                 (void**)&param,
                                 col_index+1)) {
            GetOCIDataSetError(dataset);
            return -1;
        }
        
        if(0!=SetDataSetColumn(dataset,param,pcol+col_index)) return -1;
        if(0!=SetDataSetItem(dataset,pcol+col_index,pdata+col_index,col_index+1)) return -1;
        
        col_index++;
    }

    dataset->debug.multi_fetch_rows=1;
    dataset->debug.column_count=col_count;
    return 0;
}

/*
 多行模式
 pdata得是多行模式下的数据
 */
int SetDataSetInfo_MultiRows(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count,int row_count)
{
    int col_index=0;
    OCIParam*param=(OCIParam*)0;
    
    if(!dataset||!pcol||!pdata) return -1;
    
    while(col_index<col_count) {
        if(OCI_SUCCESS!=OCIParamGet(dataset->stmt,
                                 OCI_HTYPE_STMT,
                                 dataset->err,
                                 (void**)&param,
                                 col_index+1)) {
            GetOCIDataSetError(dataset);
            return -1;
        }
        
        if(0!=SetDataSetColumn(dataset,param,pcol+col_index)) return -1;
            
        if(0!=SetDataSetItem_MultiRows(dataset,pcol+col_index,pdata+col_index,col_index+1,row_count,col_count)) return -1;
        //循环要做的一个工作是确定需要外部分配内存的尺寸，每个字段的内存位移
        //可以考虑一个字段定义一个OCIDefine*,然后每个字段设置对应的内存及位移。
        //然后将每一行对应字段的内存指针指向分配内存的正确位置，外部可以根据字段直接获取
        //再对每一个OCIDefine* 调用 OCIDefineArrayOfStruct
        //to do....
        //mark 2021-1-23.
        
        col_index++;
    }
    
    dataset->debug.multi_fetch_rows=row_count;
    dataset->debug.column_count=col_count;
    
    return 0;
}

int GetDataSetColCount(pOCIDataSet dataset,int* pcol_count)
{
    if(OCIAttrGet(dataset->stmt,
                  OCI_HTYPE_STMT,
                  (void*)pcol_count,
                  (ub4*)0,
                  (ub4)OCI_ATTR_PARAM_COUNT,
                  dataset->err)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        return -1;
    }
    
    return 0;
}

int CvtOCINum2Str(pOCIDataSet dataset,OCINumber* var_num,int precision,int scale,char* num_str)
{
    int bef_p=0,af_p=0;
    int buff_len=256;
    char format_str[256]="";
    
    for(bef_p=0;bef_p<precision-scale;bef_p++) strcat(format_str,"9");
    
    if(var_num==(OCINumber*)0) return -1;
        
    if(scale>0) {
        strcat(format_str,".");
        for(af_p=0;af_p<scale;af_p++) strcat(format_str,"9");
    }
    
    if(OCINumberToText(dataset->err,
                       var_num,
                       (oratext *)format_str,
                       strlen(format_str),
                       (oratext*)0,
                       0,
                       (ub4*)&buff_len,
                       (oratext*)num_str)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
    }
    
    return 0;
}

int CvtOCINum2Int(pOCIDataSet dataset,OCINumber* var_num,int* pvar_int)
{
    if(OCINumberToInt(dataset->err,
                      var_num,
                      sizeof(*pvar_int),
                      OCI_NUMBER_SIGNED,
                      pvar_int)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
    
    return 0;
}

int CvtOCINum2Real(pOCIDataSet dataset,OCINumber* var_num,double* pvar_double)
{
    if(OCINumberToReal(dataset->err,
                      var_num,
                      sizeof(*pvar_double),
                      pvar_double)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
    
    return 0;
}

int CvtOCIDate2Str(pOCIDataSet dataset,OCIDate* var_date,char* date_fmt,char* date_str)
{
    char date_lang[256]="simplified chinese";
    int datestr_len=256;//sizeof(date_str);
    if(OCIDateToText(dataset->err,
                     var_date,
                     (oratext*)date_fmt,
                     strlen(date_fmt),
                     (oratext*)date_lang,//(oratext*)"American",
                     strlen(date_lang),
                     (ub4*)&datestr_len,
                     (oratext*)date_str)!=OCI_SUCCESS) {
        GetOCIDataSetError(dataset);
        printf("0x%08X-%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset->error_desc);
        return -1;
    }
    return 0;
}

/*
多行模式的数据转化
 */
int SetDataSetFields_MultiRows(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count,int fetch_rows)
{
    for(int index=0;index<fetch_rows;index++) {
        for(int i_col=0;i_col<col_count;i_col++) {
            (pcol+i_col)->default_value=*((pcol+i_col)->default_list+index);
        }
        if(0!=SetDataSetFields(dataset,pcol,pdata+index*col_count,col_count)) return -1;
    }
    return 0;
}

int SetDataSetFields(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count)
{
    int i=0;//列序号
    
    if(!dataset||!pcol||!pdata) return -1;
        
    while(i<col_count) {
        //字段为空判断
        (pdata+i)->var_isnull=(((pcol+i)->default_value==-1)?1:0);
        if((pdata+i)->var_isnull) {
            if(dataset->debug.dump_method==0&&dataset->debug.level==4)
                printf("[<NULL.>]\n");
            i++;
            continue;
        }
        
        switch((pcol+i)->type) {
            case SQLT_NUM:
            {
                char num_string[256]="";
                
                if((pcol+i)->scale>0) {
                    //演示浮点数字段取值转化过程
                    double var_double=0.0;
                    if(0!=CvtOCINum2Real(dataset,&(pdata+i)->var.var_num,&var_double)) {
                        break;
                    }
                }
                
                //NUMBER子类型 Float. 其scale和precison无效，需要手工转化
                if((pcol+i)->precison==126) {
                    (pcol+i)->scale=10;//(pcol+i-1)->scale*0.30103+1;
                    (pcol+i)->precison=24;//(pcol+i-1)->precison*0.30103+1;
                }
                
                //数值转化为字符演示
                if(0!=CvtOCINum2Str(dataset,&(pdata+i)->var.var_num,(pcol+i)->precison,(pcol+i)->scale,num_string)) {
                    break;
                }
                
                if(dataset->debug.dump_method==0&&dataset->debug.level==4) printf("[%s]\n",num_string);
            }break;
            case SQLT_AFC://SQLT_AFC-asci char: char(n)
            case SQLT_CHR:
            {
                //演示字符型字段取值，存储在RDataItem的var.var_plt
                if(dataset->debug.dump_method==0&&dataset->debug.level==4)
                    printf("[%s]\n",(char*)(pdata+i)->var.var_plt);
            } break;
            case SQLT_RDD:
            {
                //演示ROWID类型取值，转化为字符存入rowid_string
                char rowid_string[256]="";
                int rowid_len=256;
                
                if(OCIRowidToChar((pdata+i)->var.var_prowid,
                                  (text*)rowid_string,
                                  (ub2*)&rowid_len,
                                  dataset->err)!=OCI_SUCCESS) {
                    break;
                }
                
                if(dataset->debug.dump_method==0&&dataset->debug.level==4)
                    printf("[%s]\n",rowid_string);               
            } break;
            case SQLT_DAT:
            {
                //演示date转字符串过程
                char date_string[256]="";
                char fmt_str[256]="yyyy-mm-dd HH24:MI:SS";
                
                if(0!=CvtOCIDate2Str(dataset,&(pdata+i)->var.var_date,fmt_str,date_string)) {
                    break;
                }
                
                if(dataset->debug.dump_method==0&&dataset->debug.level==4)
                    printf("[%s]\n",date_string);
            } break;
            case SQLT_TIMESTAMP_TZ:
            case SQLT_TIMESTAMP:
            {
                //演示timestamp转化为字符串过程
                char timestamp_string[256]="";
                char fmt_str[256]="DD-MON-RR HH.MI.SSXFF AM TZR TZD";//"yyyy-mm-dd HH24:MI:SS";
                int timestamp_len=256;
                char lang[256]="simplified chinese";
                int lang_len=strlen(lang);
                
                if(OCIDateTimeToText(dataset->connect->env,
                                     dataset->err,
                                     (pdata+i)->var.var_ptimestamp,
                                     (text*)fmt_str,
                                     (ub1)strlen(fmt_str),
                                     (ub1)5,//quote as "specifies the fractional second precision in which the
                                            //          fractional seconds is returned."
                                            //毫秒位数.
                                     (text*)lang,
                                     (ub4)lang_len,
                                     (ub4*)&timestamp_len,
                                     (text*)timestamp_string)!= OCI_SUCCESS) {
                    GetOCIDataSetError(dataset);
                    printf("%s\n",dataset->error_desc);
                    break;
                }
                
                if(dataset->debug.dump_method==0&&dataset->debug.level==4)
                    printf("[%s]\n",timestamp_string);
            } break;
            case SQLT_CLOB:
            {
                ub4 amt=0;
                ub4 offset=1;
                ub4 character_length=0;
                
                //取的是字符个数，而非字节个数
                if(OCILobGetLength(dataset->connect->svrctx,
                                dataset->err,
                                (pdata+i)->var.var_pclob,
                                &character_length)!=OCI_SUCCESS) {
                    return -1;
                }
                //双字节字符最大值
                (pdata+i)->var_pmem=calloc(character_length*2+1,sizeof(char));//缓存.
                if((pdata+i)->var_pmem==NULL) {
                    return -1;
                }
                dataset->debug.app_mem_alloc_times++;
                dataset->debug.app_mem_bytes+=character_length*2+1;
                
                if(OCILobRead(dataset->connect->svrctx,
                           dataset->err,
                           (pdata+i)->var.var_pclob,
                           (ub4*)&amt,//&buffer_size,
                           offset,
                           (void*)(pdata+i)->var_pmem,
                           (ub4)character_length*2,
                           (void*)0,
                           (OCICallbackLobRead)NULL,
                           (ub2)OCI_DEFAULT,
                           SQLCS_IMPLICIT)!=OCI_SUCCESS) {
                    return -1;
                }
                
                if(dataset->debug.dump_method==0&&dataset->debug.level==4)
                    printf("[%s]\n",(char*)(pdata+i)->var_pmem);
            } break;
        }
        
        i++;
    }
    dataset->debug.total_fetch_rows++;
    return 0;
}

pDataItem AllocateDataSetBuffer(int item_count)
{
    return (pDataItem)calloc(item_count,sizeof(RODITEM));
}

pColItem AllocateDataSetColumn(int column_count,int fetch_rows)
{
    pColItem pcol=(pColItem)calloc(column_count,sizeof(ROCOLITEM));
    if(pcol) {
        if(fetch_rows>0)
        for(int index=0;index<column_count;index++) {
            (pcol+index)->default_list=(sb2*)calloc(fetch_rows,sizeof(sb2));
        }
    }
    return pcol;
}

int SetDataSetParamByName(pOCIDataSet dataset,char* param_name,void* param_val,size_t param_val_size,ub2 param_type)
{
    OCIBind* bind=(OCIBind*)0;
    if((OCIBindByName(dataset->stmt,
                      &bind,
                      dataset->err,
                      (text*)param_name,
                      -1,//strlen(":query_id"),
                      (ub1*)param_val,
                      (sword)param_val_size, 
			 	      param_type,
			 	      (void*)0,
			 	      (ub2*)0,
			 	      (ub2)0,
			 	      (ub4)0,
			 	      (ub4*)0,
			 	      OCI_DEFAULT))!=OCI_SUCCESS) {
        return -1;
    }
    return 0;
}

void ClearDataSetBuffer(pDataItem pdata)
{
    if(pdata) {
        int column_count=_msize(pdata)/sizeof(ROCOLITEM);
        for(int index=0;index<column_count;index++) {
            unsigned short type=(pdata+index)->type;
            if(type==SQLT_AFC||type==SQLT_CHR) free((pdata+index)->var.var_plt);
            else if(type==SQLT_TIMESTAMP||type==SQLT_TIMESTAMP_TZ) OCIDescriptorFree((void*)(pdata+index)->var.var_ptimestamp,type);
            else if(type==SQLT_RDD) OCIDescriptorFree((void*)(pdata+index)->var.var_prowid,type);
            else if(type==SQLT_CLOB) {
                OCIDescriptorFree((void*)(pdata+index)->var.var_pclob,type);
                free((pdata+index)->var_pmem);
            }
        }
        free(pdata);
    }
}

void ClearDataSetColumn(pColItem pcol,int col_count)
{
    if(pcol){
        for(int index=0;index<col_count;index++) {
            if((pcol+index)->data_mem) free((pcol+index)->data_mem);
            if((pcol+index)->default_list) free((pcol+index)->default_list);
        }
        free(pcol);
    }
}

int query_test2()
{
    ROCIDataSet dataset={0};
    pColItem pcol=NULL;
    pDataItem pdata=NULL;
    
    sword code=0;
    char query_clause[1024*10]="";
    //"select rowid , a.* from t_typefield_test a";
    //"select systimestamp , rowid ,sysdate , a.* from t_04 a where rownum < 10";
    {
        FILE* file=NULL;
        char file_name[256]="t_05_005.sql";
        
        file=fopen(file_name,"rb");
        if(file) {
            int read_bytes=0;
            
            read_bytes=fread(query_clause,sizeof(char),sizeof(query_clause),file);
            if(read_bytes>=sizeof(query_clause)) {
                printf("Up to buffer max limit");
                return -1;
            }
            
            printf("query clause:\n%s\n",query_clause);
            
            fclose(file);
        }
    
    }
    int col_count=0;
    int total_row=0;
    
    SetDataSetClause(&dataset,query_clause);
    code=OpenOCIDataSet(&dataset);
    
    if(GetDataSetColCount(&dataset,&col_count)!=0) {
        printf("%s\n",dataset.error_desc);
        return -1;
    }
    printf("共计：%d列\n",col_count);
    
    pcol=(pColItem)malloc(sizeof(ROCOLITEM)*col_count);
    if(!pcol) return -1;
    
    pdata=(pDataItem)malloc(sizeof(RODITEM)*col_count);
    if(!pdata) return -1;
    
    if(SetDataSetInfo(&dataset,pcol,pdata,col_count)!=0) {
        printf("%s\n",dataset.error_desc);
        return -1;
    }
    
    while(TRUE) {
        code=OCIStmtFetch(dataset.stmt,
                          dataset.err,
                          1,
                          OCI_FETCH_NEXT,
                          OCI_DEFAULT);
        if(code==OCI_NO_DATA) {
            printf("累计:%d行\n",total_row);
            break;
        }
        else if(code==OCI_SUCCESS||code==OCI_SUCCESS_WITH_INFO) {
            if(0!=SetDataSetFields(&dataset,pcol,pdata,col_count)) {
                break;
            }
            
            total_row++;
        }
        else {
            GetOCIDataSetError(&dataset);
            printf("%s\n",dataset.error_desc);
            break;
        }
    }
    
    if(pdata) {
        for(int index=0;index<col_count;index++) {
            unsigned short type=(pdata+index+1)->type;
            if(type==SQLT_AFC||type==SQLT_CHR) free((pdata+index+1)->var.var_plt);
            else if(type==SQLT_TIMESTAMP||type==SQLT_TIMESTAMP_TZ) OCIDescriptorFree((void*)(pdata+index+1)->var.var_ptimestamp,type);
            else if(type==SQLT_RDD) OCIDescriptorFree((void*)(pdata+index+1)->var.var_prowid,type);
        }
        free(pdata);
    }
    if(pcol) free(pcol);
    
    CloseOCIDataSet(&dataset);
        
    //ReleaseOCIConnection(GetOCIConnection());
    
    return 0;
}

int WINAPI query_test_batchfetch(LPVOID param)
{   
    ROCIDataSet dataset={0};
    pColItem pcol=NULL;
    pDataItem pdata=NULL;
    
    sword code=0;
    char query_clause[1024*10]="select * from t_04 where rownum < 100000";
    
    int col_count=0;
    int total_row=0;
    int multi_fetch_rows=1000;
    
    //debug
    //dataset.debug.level=4;
    dataset.debug.dump_method=0;
    
    printf("0x%08X:线程进入\n",GetCurrentThreadId());
    
    if(SetDataSetClause(&dataset,query_clause)!=0) {
        printf("0x%08X:加载SQL失败，线程退出(×)\n",GetCurrentThreadId());
        goto thread_quit;
    }
    
    if(0!=OpenOCIDataSet(&dataset)) {
        printf("0x%08X:打开数据集失败，线程退出(×)\n",GetCurrentThreadId());
        goto thread_quit;
    }
    
    if(GetDataSetColCount(&dataset,&col_count)!=0) {
        printf("0x%08X/%s:%s\n",GetCurrentThreadId(),__FUNCTION__,dataset.error_desc);
        printf("0x%08X:获取数据集失败，线程退出(×)\n",GetCurrentThreadId());
        goto thread_quit;
    }
    //printf("共计：%d列\n",col_count);
    
    pcol=AllocateDataSetColumn(col_count,multi_fetch_rows);
    if(!pcol) {
        printf("0x%08X:分配数据集表头缓存失败，线程退出(×)\n",GetCurrentThreadId());
        goto thread_quit;
    }
    
    pdata=AllocateDataSetBuffer(col_count*multi_fetch_rows);
    if(!pdata) {
        printf("0x%08X:分配数据集缓存失败，线程退出(×)\n",GetCurrentThreadId());
        goto thread_quit;
    }
    
    if(SetDataSetInfo_MultiRows(&dataset,pcol,pdata,col_count,multi_fetch_rows)!=0) {
        printf("0x%08X:%s，绑定数据集缓存失败，线程退出....\n",GetCurrentThreadId(),dataset.error_desc);
        goto thread_quit;
    }
    
    printf("0x%08X:线程正进入fetch循环\n",GetCurrentThreadId());
    
    int c_fetch_rows=0;
    while(TRUE) {
        code=OCIStmtFetch(dataset.stmt,
                          dataset.err,
                          multi_fetch_rows,
                          OCI_FETCH_NEXT,
                          OCI_DEFAULT);
        OCIAttrGet(dataset.stmt,OCI_HTYPE_STMT,&c_fetch_rows,(ub4*)NULL,OCI_ATTR_ROWS_FETCHED,dataset.err);
        total_row+=c_fetch_rows;
        
        if(code==OCI_NO_DATA) {
            if(0!=SetDataSetFields_MultiRows(&dataset,pcol,pdata,col_count,c_fetch_rows)) {
                break;
            }
            break;
        }
        else if(code==OCI_SUCCESS||code==OCI_SUCCESS_WITH_INFO) {
            if(0!=SetDataSetFields_MultiRows(&dataset,pcol,pdata,col_count,c_fetch_rows)) {
                break;
            } 
        }
        else {
            GetOCIDataSetError(&dataset);
            printf("0x%08X:遍历数据失败，%s\n",GetCurrentThreadId(),dataset.error_desc);
            goto thread_quit;
        }
    }
    
    printf("0x%08X:线程退出(√)\n",GetCurrentThreadId());
    
thread_quit:
    CloseOCIDataSet(&dataset);
    
    ClearDataSetBuffer(pdata);
    ClearDataSetColumn(pcol,col_count);
    return 0;
}

void main()
{
    //200个线程可能过多，导致WaitForXXX失效.
    HANDLE thread_list[200];
    
    AllocateConnPool("YSVR","test","test");
    query_test2();
    printf("\n\n");
    
    printf("正在分配任务线程(%d),最大连接数(%d),并执行...\n",sizeof(thread_list)/sizeof(thread_list[0]),OCI_CONN_MAXCOUNT);
    for(int index=0;index<sizeof(thread_list)/sizeof(thread_list[0]);index++)
    {
        unsigned int threadid=0;
        thread_list[index]=(HANDLE)_beginthreadex(NULL,0,(_beginthreadex_proc_type)query_test_batchfetch,NULL,0,&threadid);    
    }
    
    WaitForMultipleObjects(sizeof(thread_list)/sizeof(thread_list[0]),thread_list,TRUE,INFINITE);
    printf("done.\n");
    
    for(int index=0;index<sizeof(thread_list)/sizeof(thread_list[0]);index++) CloseHandle(thread_list[index]);
    
    //测试一下oracle的session到底有多少个
    //select a.program , a.* from v$session a where a.username='%s',app_name//...
    Sleep(1000*15);
    
    ClearConnPool();
}
/*******************************


int query_test(char* svr,char* usr,char* pwd)
{
    ROCIDataSet dataset={0};
    OCIParam*param=(OCIParam*)0;
    OCIDefine* pdef=(OCIDefine*)0;
    pColItem pcol=NULL;
    pDataItem pdata=NULL;
    
    sword code=0;
    char query_clause[1024]="select rowid , a.* from t_typefield_test a";
    int col_count=0,col_index=1,row_index=1;
    //int pre_allot_row_count=0;//预分配约10M内存
    ub4 col_name_len;
    sb2 default_value=0;
    
    int total_row=0;
    
    InitialOCIConnection(svr,usr,pwd);
    
    SetDataSetClause(&dataset,query_clause);
    code=OpenOCIDataSet(&dataset);
    
    OCIAttrGet(dataset.stmt,
               OCI_HTYPE_STMT,
               (void*)&col_count,
               (ub4*)0,
               (ub4)OCI_ATTR_PARAM_COUNT,
               dataset.err);
    printf("共计：%d列\n",col_count);
    pcol=(pColItem)malloc(sizeof(ROCOLITEM)*col_count);
    
    //pre_allot_row_count=10*1024*1024/sizeof(ROCOLITEM)/col_count;
    pdata=(pDataItem)malloc(sizeof(RODITEM)*col_count);
        
    while(col_index<=col_count) {
        if(OCI_SUCCESS!=OCIParamGet(dataset.stmt,
                                 OCI_HTYPE_STMT,
                                 dataset.err,
                                 (void**)&param,
                                 col_index)) break;
        
        
        ub4 type_len=sizeof(unsigned short);
        ub4 isnull_len=sizeof(unsigned short);
        ub4 size_len=sizeof(int);
        text* pcol_name=&(pcol+col_index-1)->col_name[0];
        col_name_len=256;//sizeof(pcol_name);//sizeof(pcol_name);//((pcol+col_index-1)->col_name);
        
        if(OCIAttrGet((void*)param,
                      (ub4)OCI_DTYPE_PARAM,
                      (void**)&pcol_name,
                      (ub4*)&col_name_len,
                      (ub4)OCI_ATTR_NAME,
                      dataset.err)!=OCI_SUCCESS) {
            GetOCIDataSetError(&dataset);
            printf("%s\n",dataset.error_desc);
        }
         
        OCIAttrGet((void*)param,
                   (ub4)OCI_DTYPE_PARAM,
                   (void*)&(pcol+col_index-1)->type,
                   (ub4*)&type_len,
                   (ub4)OCI_ATTR_DATA_TYPE,
                   dataset.err);
        
        OCIAttrGet((void*)param,
                   (ub4)OCI_DTYPE_PARAM,
                   (void*)&(pcol+col_index-1)->isnull_flag,
                   (ub4*)&isnull_len,
                   (ub4)OCI_ATTR_IS_NULL,
                   dataset.err);
                   
        (pcol+col_index-1)->size=\
        (pcol+col_index-1)->scale=\
        (pcol+col_index-1)->precison=0;
        OCIAttrGet((void*)param,
                   (ub4)OCI_DTYPE_PARAM,
                   (void*)&(pcol+col_index-1)->size,
                   (ub4*)&size_len,
                   (ub4)OCI_ATTR_DATA_SIZE,
                   dataset.err);
                   
        OCIAttrGet((void*)param,
                   (ub4)OCI_DTYPE_PARAM,
                   (void*)&(pcol+col_index-1)->scale,
                   (ub4*)&size_len,
                   (ub4)OCI_ATTR_SCALE,
                   dataset.err);
                   
        OCIAttrGet((void*)param,
                   (ub4)OCI_DTYPE_PARAM,
                   (void*)&(pcol+col_index-1)->precison,
                   (ub4*)&size_len,
                   (ub4)OCI_ATTR_PRECISION,
                   dataset.err);
                
        //经查 float是 number类型的子类型，precison[1~126],scale不可自定义   
        printf("%s %d-%d(%d,%d) %d\n",pcol_name,//(pcol+col_index-1)->col_name,
                                (pcol+col_index-1)->type,
                                (pcol+col_index-1)->size,
                                (pcol+col_index-1)->precison,
                                (pcol+col_index-1)->scale,
                                (pcol+col_index-1)->isnull_flag);
        
        //绑定输出
        switch((pcol+col_index-1)->type) {
        case SQLT_RDD:
        {
            //rowid.
            //OCIDefineByPos(dataset.stmt,
            //               &pdef,
            //               dataset.err,
            //               col_index,
            //               (dvoid*)&((pdata+col_index-1)->var.var_rowid),
            //               (sword)(pcol+col_index-1)->size,
            //               (pcol+col_index-1)->type,
            //               (dvoid*)(((pcol+col_index-1)->isnull_flag!=0)?&default_value:0),
            //               (ub2*)0,
            //               (ub2*)0,
            //               OCI_DEFAULT);
        } break;
        case SQLT_AFC:
        {
            //SQLT_AFC-asci char: char(n)
            
        } break;
        case SQLT_DAT:
        {
            //SQLT_DAT : date
            OCIDefineByPos(dataset.stmt,
                           &pdef,
                           dataset.err,
                           col_index,
                           (dvoid*)&(pdata+col_index-1)->var.var_date,
                           (sword)(pcol+col_index-1)->size,
                           SQLT_ODT,//(pcol+col_index-1)->type, //3天。。。
                           (dvoid*)(((pcol+col_index-1)->isnull_flag!=0)?&default_value:0),
                           (ub2*)0,
                           (ub2*)0,
                           OCI_DEFAULT);
        } break;
        case SQLT_TIMESTAMP:
        {
            //SQLT_TIMESTAMP : timestamp
            OCIDefineByPos(dataset.stmt,
                           &pdef,
                           dataset.err,
                           col_index,
                           (dvoid*)&(pdata+col_index-1)->var.var_date,
                           (sword)(pcol+col_index-1)->size,
                           SQLT_ODT,
                           (dvoid*)(((pcol+col_index-1)->isnull_flag!=0)?&default_value:0),
                           (ub2*)0,
                           (ub2*)0,
                           OCI_DEFAULT);
        } break;
        case SQLT_CHR:
            (pdata+col_index-1)->var.var_plt=malloc(sizeof(char)*((pcol+col_index-1)->size+1));
            memset((pdata+col_index-1)->var.var_plt,0x00,_msize((pdata+col_index-1)->var.var_plt)/sizeof(char));
            OCIDefineByPos(dataset.stmt,
                           &pdef,
                           dataset.err,
                           col_index,
                           (dvoid*)(pdata+col_index-1)->var.var_plt,
                           (sword)(pcol+col_index-1)->size,
                           (pcol+col_index-1)->type,
                           (dvoid*)(((pcol+col_index-1)->isnull_flag!=0)?&default_value:0),
                           (ub2*)0,
                           (ub2*)0,
                           OCI_DEFAULT);
            break;
        case SQLT_NUM:
        {
            //(pdata+col_index-1)->var.var_plt=malloc(sizeof(char)*256);
            OCIDefineByPos(dataset.stmt,
                           &pdef,
                           dataset.err,
                           col_index,
                           (dvoid*)&((pdata+col_index-1)->var.var_num),
                           (sword)(pcol+col_index-1)->size,
                           SQLT_VNU,//(pcol+col_index-1)->type,
                           (dvoid*)(((pcol+col_index-1)->isnull_flag!=0)?&default_value:0),
                           (ub2*)0,
                           (ub2*)0,
                           OCI_DEFAULT);
        } break;
        default:
            printf("OCI data type:%d,required modification.\n",
                    (pcol+col_index-1)->type);
        }
        col_index++;
    }
    
    code=OCIStmtFetch(dataset.stmt,
                      dataset.err,
                      1,
                      OCI_FETCH_NEXT,
                      OCI_DEFAULT);
    while(TRUE) {
        if(code==OCI_NO_DATA) {
            printf("累计:%d行\n",total_row);
            break;
        }
        else if(code==OCI_SUCCESS||code==OCI_SUCCESS_WITH_INFO) {
            for(int i=0;i<col_count;i++) {
                switch((pcol+i-1)->type) {
                case SQLT_NUM:
                {
                    if((pcol+i-1)->scale==0) {
                        int var_int;
                        OCINumberToInt(dataset.err,
                                       &(pdata+i-1)->var.var_num,
                                       sizeof(var_int),
                                       OCI_NUMBER_SIGNED,
                                       &var_int);
                        (pdata+i-1)->var.var_int=var_int;
                        printf("(%d,%d)%d\n",(pcol+i-1)->precison,(pcol+i-1)->scale,(pdata+i-1)->var.var_int);
                    }
                    else {
                        int buff_len=256;
                        char num_str[256]="";
                        char format_str[256]="";
                        
                        if((pcol+i-1)->precison==126) {
                            //NUMBER子类型 float
                            //相关实现有缺陷
                            (pcol+i-1)->scale=10;//(pcol+i-1)->scale*0.30103+1;
                            (pcol+i-1)->precison=24;//(pcol+i-1)->precison*0.30103+1;
                        }
                        
                        for(int index=0;index<(pcol+i-1)->precison-(pcol+i-1)->scale;index++) {
                            strcat(format_str,"9");
                        }
                        if((pcol+i-1)->scale>0) {
                            strcat(format_str,".");
                            for(int index=0;index<(pcol+i-1)->scale;index++)
                                strcat(format_str,"9");
                        }
                        
                        if(OCINumberToText(dataset.err,
                                          &(pdata+i-1)->var.var_num,
                                          (oratext *)format_str,
                                          strlen(format_str),
                                          (oratext*)0,
                                          0,
                                          (ub4*)&buff_len,
                                          (oratext*)num_str)!=OCI_SUCCESS) {
                            GetOCIDataSetError(&dataset);
                            printf("%s\n",dataset.error_desc);
                        }
                        
                        printf("(%d,%d)%s-%s\n",(pcol+i-1)->precison,(pcol+i-1)->scale,format_str,num_str);
                    }
                }break;
                case SQLT_CHR:
                {
                    printf("(%d,%d):[%s]\n",(pcol+i-1)->precison,(pcol+i-1)->scale,(pdata+i-1)->var.var_plt);
                } break;
                case SQLT_RDD:
                {
                    //rowid.
                    //OCIRowidToStr();
                    
                } break;
                case SQLT_AFC:
                {
                    //SQLT_AFC-asci char: char(n)
                    
                } break;
                case SQLT_DAT:
                {
                    //SQLT_DAT : date
                    char date_fmt[256]="yyyy-mm-dd HH24:MI:SS";//"YYYYMMDD HH24:MI:SS";// 
                    char date_str[256]="";
                    char date_lang[256]="simplified chinese";
                    int datestr_len=sizeof(date_str);
                    if(OCIDateToText(dataset.err,
                                     &(pdata+i-1)->var.var_date,
                                     (oratext*)date_fmt,
                                     strlen(date_fmt),
                                     (oratext*)date_lang,//(oratext*)"American",
                                     strlen(date_lang),
                                     (ub4*)&datestr_len,
                                     (oratext*)date_str)!=OCI_SUCCESS) {
                        GetOCIDataSetError(&dataset);
                        printf("%s\n",dataset.error_desc);
                    }
                    printf("(%d,%d):[%s]\n",(pcol+i-1)->precison,(pcol+i-1)->scale,date_str);
                } break;
                case SQLT_TIMESTAMP:
                {
                    //SQLT_TIMESTAMP : timestamp
                    
                    char datestamp_fmt[256]="yyyy-mm-dd HH24:MI:SS";
                    char datestamp_str[256]="";
                    char datestamp_lang[256]="simplified chinese";
                    int datestamp_len=sizeof(datestamp_str);
                    if(OCIDateToText(dataset.err,
                                     &(pdata+i-1)->var.var_date,
                                     (oratext*)datestamp_fmt,
                                     strlen(datestamp_fmt),
                                     (oratext*)datestamp_lang,//(oratext*)"American",
                                     strlen(datestamp_lang),
                                     (ub4*)&datestamp_len,
                                     (oratext*)datestamp_str)!=OCI_SUCCESS) {
                        GetOCIDataSetError(&dataset);
                        printf("%s\n",dataset.error_desc);
                    }
                    printf("(%d,%d):[%s]\n",(pcol+i-1)->precison,(pcol+i-1)->scale,datestamp_str);
                } break;
                }
            }
            
            printf("%d:%ld,%s\n",total_row,(pdata+7)->var.var_int,(pdata+1)->var.var_plt);
            
            code=OCIStmtFetch(dataset.stmt,
                              dataset.err,
                              1,
                              OCI_FETCH_NEXT,
                              OCI_DEFAULT);
            total_row++;
        }
        else {
            GetOCIDataSetError(&dataset);
            printf("%s\n",dataset.error_desc);
            break;
        }
    }
    
    CloseOCIDataSet(&dataset);
    
    if(pcol) free(pcol);
    if(pdata) free(pdata);
    
    ReleaseOCIConnection(GetOCIConnection());
    
    return 0;
}

*/