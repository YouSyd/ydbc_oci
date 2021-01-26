#include "ocidbtools.h"


/*
 syslistview32的每一项如果绑定特定的userdata,假设一个字段32字节(类型，数值，...)，如果有100个字段，一行3200bytes,约3K
 1万行即30M，这个内存的涨落有点大。
 考虑在设计对接oci查询的数据接口时，提供一个选项：
 1、无附加数据，从数据库取出就用，会省下空间，且速度很快
 2、挂载附加数据，但是对数据记录数进行限制，比如附加数据大于10M给与提示，加载策略
 */

typedef struct __STRUCT_OCIDATAITEM__ {
    unsigned short type;
    union value{
        void* var_plt;
        double var_double;
        float var_float;
        int var_int;
        
        OCINumber var_num;
        OCIDate var_date;
        
        //oci接口里头有，而oci的头文件里头没找到定义...
        //OCITime var_timestamp;
        OCIRowid* var_prowid;//指针可用，但是实例报错
        OCIDateTime* var_ptimestamp;//
        OCILobLocator* var_pclob;//CLob
        //....
        //后续完善
    } var;
    void* var_pmem;//存储CLob的数据缓冲
    unsigned short var_isnull;
    size_t size;
    
}RODITEM,RDataItem,*pDataItem;

typedef struct __STRUCT_OCICOL_ITEM__ {
    //unsigned short col_index;
    unsigned short type;
    text col_name[256];
    unsigned short isnull_flag;
    int size;
    int scale;
    int precison;
    sb2 default_value;
    
    OCINumber var_sum;//统计数据
    char sum_text[256];//统计数据字符转化
 
    //2021-1-23 新增多行读取
    OCIDefine* pdef;
    void* data_mem;
    sb2* default_list;//多行模式用于检测是否空值的列表
       
    int col_item_width;//自适应列宽
}ROCOLITEM,RColItem,*pColItem;

/********    OCIDefineByPos   *********/
int SetDataSetItem(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_pos);

/********    OCIAttrGet       ********/ //获取列属性
int SetDataSetColumn(pOCIDataSet dataset,OCIParam*param,pColItem pcol);

/********    OCIAttrGet+OCIDefineByPos   *********/
int SetDataSetInfo(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count);
int SetDataSetInfo_MultiRows(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count,int row_count);

/********    OCIAttrGet/OCI_ATTR_PARAM_COUNT    *******/ //获取列数量
int GetDataSetColCount(pOCIDataSet dataset,int* pcol_count);

/********    例子程序，用于展示将OCI类型转化为C基础类型 *********/
int SetDataSetFields(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count);

/********    多行的数据集操作 *************/
int SetDataSetFields_MultiRows(pOCIDataSet dataset,pColItem pcol,pDataItem pdata,int col_count,int fetch_rows);

int SetDataSetParamByName(pOCIDataSet dataset,char* param_name,void* param_val,size_t param_val_size,ub2 param_type);

pDataItem AllocateDataSetBuffer(int item_count);
pColItem AllocateDataSetColumn(int column_count,int fetch_rows);
void ClearDataSetBuffer(pDataItem pdata);
void ClearDataSetColumn(pColItem pcol);

int CvtOCINum2Str(pOCIDataSet dataset,OCINumber* var_num,int precision,int scale,char* num_str);
int CvtOCINum2Int(pOCIDataSet dataset,OCINumber* var_num,int* pvar_int);
int CvtOCINum2Real(pOCIDataSet dataset,OCINumber* var_num,double* pvar_double);
int CvtOCIDate2Str(pOCIDataSet dataset,OCIDate* var_date,char* date_fmt,char* date_str);

#define OCI_NUM2STR(dataset,var_num/*OCINumber*/,col/*pColItem*/,buffer) \
{\
    if((col)->scale>0) {\
        double __var_double__=0.0;\
        char __fmt_str__[256]="";\
        sprintf(__fmt_str__,"%%.%dlf",(col)->scale);\
        CvtOCINum2Real((dataset),&(var_num),&__var_double__);\
        sprintf(buffer,__fmt_str__,__var_double__);\
    }\
    else {\
        char __buf__[256]="";\
        CvtOCINum2Str((dataset),&(var_num),(col)->precison,(col)->scale,__buf__);\
        for(int __index__=strlen(__buf__)-1;__index__>=0;__index__--) if(__buf__[__index__]==' ') {strcpy(buffer,&__buf__[__index__+1]);break;}\
    }\
}

#define OCI_VAR2STR(dataset,field/*pDataItem*/,col/*pColItem*/,buffer) \
{\
    if((field)->var_isnull) {\
        buffer[0]='\0';\
    }\
    else {\
    switch(col->type) {\
    case SQLT_NUM: {\
        OCI_NUM2STR(dataset,((field)->var.var_num),col,buffer);\
    } break;\
    case SQLT_AFC: \
    case SQLT_CHR: {\
        strcpy(buffer,(char*)(field)->var.var_plt);\
        for(int __index__=strlen(buffer)-1;__index__>=0;__index__--) if(buffer[__index__]==' ') buffer[__index__]='\0';\
    } break;\
    case SQLT_RDD: {\
        int __rowid_len__=256;\
        OCIRowidToChar((field)->var.var_prowid,(text*)buffer,(ub2*)&__rowid_len__,(dataset)->err); \
    } break;\
    case SQLT_DAT: {\
        char __fmt_str__[256]="yyyy-mm-dd HH24:MI:SS";\
        CvtOCIDate2Str((dataset),&(field)->var.var_date,__fmt_str__,buffer);\
    } break;\
    case SQLT_TIMESTAMP_TZ:\
    case SQLT_TIMESTAMP: {\
        char __fmt_str__[256]="DD-MON-RR HH.MI.SSXFF AM TZR TZD";\
        int __timestamp_len__=256;\
        char __lang__[256]="simplified chinese";\
        int __lang_len__=strlen(__lang__);\
        OCIDateTimeToText((dataset)->connect->env,\
                          (dataset)->err,\
                          (field)->var.var_ptimestamp,\
                          (text*)__fmt_str__,\
                          (ub1)strlen(__fmt_str__),\
                          (ub1)5,\
                          (text*)__lang__,\
                          (ub4)__lang_len__,\
                          (ub4*)&__timestamp_len__,\
                          (text*)buffer);\
    } break;\
    }\
    }\
}

/*宏定义变量的bug*/
        /*if(col->scale>0) {\
            double var_double=0.0;\
            char fmt_str[256]="";\
            sprintf(fmt_str,"%%.%dlf",col->scale);\
            CvtOCINum2Real((dataset),&(field)->var.var_num,&var_double);\
            sprintf(buffer,fmt_str,var_double);\
        }\
        else {\
            char buf[256]="";\
            CvtOCINum2Str((dataset),&(field)->var.var_num,(col)->precison,(col)->scale,buf);\
            for(int index=strlen(buf)-1;index>=0;index--) if(buf[index]==' ') {strcpy(buffer,&buf[index+1]);break;}\
        }*/\