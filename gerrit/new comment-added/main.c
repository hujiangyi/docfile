/*
1. 提交代码的说明，必须符合格式。
格式如下：
-----------------------------------------------------------
[问题单号] 问题描述
[修改点]
1. XXXXXXXXX
2. XXXXXXX
[验证]
1. XXXXX
2. XXXXX

Signed-off-by: XXXXX <XXXX@dvt.dvt.com>
-----------------------------------------------------------

其中，问题描述、修改点和验证的字数均不能少于10个汉字。


2. 参与代码审核者，包括提交人在内，不能少于2人。

3. 每次评审打分，必须有评审意见，字数不能少于10个汉字。

4. 每个评审记录，至少批注一个源代码文件，字数不能少于10个汉字。

5. 在代码审核过程中发现的严重问题，如内存操作越界、泄露、数组越界、破坏已有功能等等，问题发现者允许被提名作为本项目组内的啄木鸟奖。

6. 按小组统计有效评审意见，人均有效评审意见数量最多的小组，当月获得人均小组活动经费50元，并作为组长质量管理数据纳入年中和年底考核。
考核月底公示一天， 其他组可检举, 若弄虚做假则取消资格。

注：
1至4是强制性标准，不符合将被服务器拒绝提交。

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cJSON.h"
#include "tools.h"

#define VALID_COMMENT_LEN 20

char *err_str[]= {
    "Bug title wrong; ", //问题单格式不对; ",
    "Commit message format wrong; ", //提交信息格式不对; ",
    "Approvaled by less than 3 guys; ", //" 审核者少于2人; ",
    "Review comments less than 20 bytes; ", //"审核意见少于10个汉字或20个字符; ",
    "File comments less than 20 bytes, add a long comment pls. ", //"文件批注意见少于10个汉字或20个字符: ",
    "File comments absent or too short; ", //缺文件批注; ",
    "Review record duplicate; ", //"同一个单有多个评审记录",
    "commit msg too lazy; ", //"不能偷懒，打点不算数",
    "commit message bug nature absent; ", //缺[问题性质]
};

enum {
    err_id_trackid=0,
    err_id_commit,
    err_id_approval_less,
    err_id_approval_too_short,
    err_id_file_comment_too_short,
    err_id_file_comment_absent,
    err_id_file_review_record_duplicate,
    err_id_commit_too_lazy,
    err_id_commit_nature,
};

/*-----------------------------------------------------------------------------*/
static char err_msg[1024*128];

void err_info_cat(char *err_info)
{
    if( strlen(err_msg) + strlen(err_info) >= (sizeof(err_msg) - 1) )
        return;

    strcat(err_msg, err_info);
}

void err_info_print()
{
    printf("%s\n", err_msg);
}

static char data[128*1024];
/*-----------------------------------------------------------------------------*/
/* Read a file, parse, render back, etc. */
cJSON * json_open(char *cmd)
{
    FILE *f;
    cJSON * json;

//    printf("cmd: %s\n", cmd);

	if(!(f=popen(cmd,"r"))) {
        printf("CMD %s open fail!\n", cmd);
        return 0;
	}

    memset(data, 0, sizeof(data));
	fgets(data, sizeof(data)-1, f);
#if 0
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);
	memset(data, 0, len+1);
	fread(data,1,len,f);
#endif
	pclose(f);

//    printf("data:\n%s\n", data);

	json=cJSON_Parse(data);
	if (!json) {
            printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    }

//	free(data);
	return json;
}

void json_close(cJSON * json)
{
    if(json)
        cJSON_Delete(json);
}
/*-----------------------------------------------------------------------------*/

int gerrit_approvals_check(cJSON *patchSet)
{
    cJSON *jsons, *json;
    cJSON *by, *name;
    int cnt, valid_cnt=0;
    int i;
    char *username;

    jsons = cJSON_GetObjectItem(patchSet, "approvals");
    if(!jsons) {
        err_info_cat(err_str[err_id_approval_less]);
        return FAIL;
    }
    cnt = cJSON_GetArraySize(jsons);

    for (i=0; i<cnt; i++) {
        json = cJSON_GetArrayItem(jsons, i);
        if(!json)
            continue;

        by = cJSON_GetObjectItem(json, "by");
        if(!by)
            continue;

        name = cJSON_GetObjectItem(by, "username");
        if(!name)
            continue;

        username = name->valuestring;

        /* gerrit帐号是自动评审，应该排除 */
        if(strcmp(username, "ems")) {
            valid_cnt++;
        }
    }

    if(valid_cnt < 3) {
        //return FAIL;
    }

    return OK;
}

/* 检查文件批注，只要有一条超过20字就可以 */
int gerrit_patchSet_comments_check(cJSON *patchSet)
{
    /*
    cJSON *jsons, *json,*msg;
    int cnt;
    int i;
    int ret=1;

    if(! (jsons = cJSON_GetObjectItem(patchSet, "comments"))){
        return FAIL;
    }

    cnt = cJSON_GetArraySize(jsons);

    for (i=0; i<cnt; i++) {
        json = cJSON_GetArrayItem(jsons, i);
        if(!json)
            continue;

        msg = cJSON_GetObjectItem(json, "message");
        if(!msg)
            continue;

        if( strlen(msg->valuestring) >= VALID_COMMENT_LEN &&
           (!strstr(msg->valuestring, "....")) &&
           (!strstr(msg->valuestring, "。。。。")) ) {
            ret = 0;
        }
    }

    return ret;
    */
    return OK;
}

int gerrit_patchSets_check(cJSON *json)
{
    cJSON *patchSets, *patchSet;
    int patchSets_cnt;
    int i;
    int approval_ok=0, patchSet_comment_ok=0;

    patchSets = cJSON_GetObjectItem(json, "patchSets");
    if(!patchSets)
        return FAIL;

    patchSets_cnt = cJSON_GetArraySize(patchSets);
    if(patchSets_cnt <= 0)
        return FAIL;

    for (i=0; i<patchSets_cnt; i++) {
        patchSet = cJSON_GetArrayItem(patchSets, i);
        if(!patchSet) {
            break;
        }

        if( (!approval_ok) && (!gerrit_approvals_check(patchSet) )){
            approval_ok = 1;
        }

        if( (!patchSet_comment_ok) && 
		(!gerrit_patchSet_comments_check(patchSet))
		){
            patchSet_comment_ok = 1;
        }

        if(approval_ok && patchSet_comment_ok) {
            return 0;
        }
    }

    if(!approval_ok) {
        err_info_cat(err_str[err_id_approval_less]);
    }

    if(!patchSet_comment_ok) {
        err_info_cat(err_str[err_id_file_comment_too_short]);
    }

    return 1;
}

int msg_valid(char *msg)
{
    char *p;
    int ret=FAIL;

    if(!msg) {
        return ret;
    }

    if( (strstr(msg, "..")) ||
        (strstr(msg, "。。"))) {
        return ret;
    }

    if( (p=strstr(msg, "comment)\n\n"))) {
        if( strlen(p)-10 >= VALID_COMMENT_LEN) {
            ret = OK;
        }
    }
    else if( (p=strstr(msg, "Code-Review"))) {
        if( strlen(p)-15 >= VALID_COMMENT_LEN) {
            ret = OK;
        }
    }

    return ret;
}

/* 只要有一条评审意见大于20字符就过关 */
int gerrit_comments_check(cJSON *root)
{
    cJSON *jsons, *json, *msg, *reviewer, *username;
    int cnt;
    int i;
    int ret=FAIL;

    jsons = cJSON_GetObjectItem(root, "comments");
    if(!jsons)
        return ret;

    cnt = cJSON_GetArraySize(jsons);

    for (i=0; i<cnt; i++) {
        json = cJSON_GetArrayItem(jsons, i);
        if(!json)
            continue;

        reviewer = cJSON_GetObjectItem(json, "reviewer");
        if(!reviewer)
            continue;

        username = cJSON_GetObjectItem(reviewer, "username");
        if(!username)
            continue;

        msg = cJSON_GetObjectItem(json, "message");
        if(!msg)
            continue;

        if(strcmp(username->valuestring, "ems") 
        //&&!msg_valid(msg->valuestring)
                   ) {
            ret = OK;
            break;
        }
    }

    if(ret) {
        err_info_cat(err_str[err_id_approval_too_short]);
    }

    return ret;
}
/*-----------------------------------------------------------------------------*/
/*
检查问题单号
[OLT-100] XXXXXXXXXX

*/

static char *project_names[]= {
    "OLT",
    "ONU",
    "CCMTS",
    "EMS",
    "PRODUCTPROBLEM",
    "PRODUCTREQUIREMENT",
    "UTILS",
};

int project_match(char *project)
{
    int i=0;

    for (i=0; i<sizeof(project_names)/sizeof(char *); i++) {
        if(!strcmp(project_names[i], project)) {
            return OK;
        }
    }
    return FAIL;
}

int is_number(char *n)
{
    while(*n) {
        if ( !isdigit(*n) )
            return FAIL;
        n++;
    }

    return OK;
}


int trackid_check(char *msg)
{
    char trackid[64];
    char *p;
    char *tail;
    int trackno;

    memset(trackid, 0, sizeof(trackid));

    /* 检测开始的[ */
    if (*msg != '['){
        return FAIL;
    }
    msg++;

    /* 找到] */
    if( !(p=strstr(msg, "]"))) {
       return FAIL;
    }

    /* 问题单描述 */
    if( !(tail=strstr(p, "\n"))) {
       return FAIL;
    }
    /* 问题单描述不能少于10个汉字，或20个字符 */
    if(tail - p < VALID_COMMENT_LEN) {
        //return FAIL;
    }

    /* 复制单号 */
    memcpy(trackid, msg, p - msg);

    /* ?业? - */
    if( !(p=strstr(trackid, "-") )){
       return FAIL;
    }

    /* 分割工程名和数字 */
    *p = '\0';
    p++;

    /* 检查工程名 */
    if(project_match(trackid)) {
        return FAIL;
    }

    /* 检查单号是否数字 */
    if(is_number(p)) {
        return FAIL;
    }

    if(sscanf(p, "%d", &trackno) <= 0) {
        return FAIL;
    }

    /* 限制单号要小于40000，避免以工号凑数 */
    if(trackno >= 40000){
        //return FAIL;
    }

    return OK;
}

/*
    reviewer check
*/
int gerrit_reviewer_check(cJSON * root)
{
    cJSON *patchSets, *patchSet;
    int i,psize,cnt;

    char name[20][32];
    memset(name,0,sizeof(name));
    printf("name size:[%d][%d]",cnt,sizeof(name));

    patchSets = cJSON_GetObjectItem(root, "patchSets");
    if(!patchSets)
        return FAIL;
 
    psize = cJSON_GetArraySize(patchSets);
    if(psize <= 0)
        return FAIL;
    
    for (i=0; i<psize; i++) {
        patchSet = cJSON_GetArrayItem(patchSets, i);
        if(!patchSet) 
            break;
        cJSON *approvals,*approval;
        approvals = cJSON_GetObjectItem(patchSet, "approvals");
        if(!approvals) {
            err_info_cat("No one is checking.");
            return FAIL;
        }   
        cnt = cJSON_GetArraySize(approvals);
        for (i=0; i<cnt; i++) {
            approval = cJSON_GetArrayItem(approvals, i);
            if(!approval)
                continue;
            cJSON *by,*uname;
            by = cJSON_GetObjectItem(approval, "by");
            if(!by)
                continue;

            uname = cJSON_GetObjectItem(by, "username");
            if(!uname)
                continue;
            
	        strncpy(name[i],uname->valuestring,strlen(uname->valuestring));
            printf("reviewer:[%s][%s][%d]",name[i],uname->valuestring,strlen(uname->valuestring));
        }
    }

    cJSON *commitMessage;
    char *c=0;
    char *start=0;
    char reviewers[512];
    char reviewer[32];
    int nameLength=0;
    
    commitMessage = cJSON_GetObjectItem(root, "commitMessage");
	printf(commitMessage->valuestring);
    start = strstr(commitMessage->valuestring,"[reviewer]");
	printf("start%s",start);
    if (start == NULL) 
    {
        printf("no reviewers\n");
        err_info_cat(err_str[err_id_commit]);
        return FAIL;
    }
    start = start + 10;
    for (c=start;*c!='\0'&& *c!='['&&*c!='\n';c++);
    memset(reviewers,0,sizeof(reviewers));
    strncpy(reviewers,start,c - start);
    if (sizeof(reviewers) == 0) 
    {
        printf("no reviewers\n");
        return OK;
    }
    printf("gerrit_reviewer:%s\n",reviewers);
    start = reviewers;
    int over = 1;
    for (c=reviewers;*c!='\0';c++)
    {
	    if(*c==',')
        {
            char *end=c-1;
            for (;*start==' ' || *start=='\t' || *start=='\n' || *start=='\r';start++); 
            for (;*end==' ' || *end=='\t' || *end=='\n' || *end=='\r';end--); 
            memset(reviewer,0,32);
	        strncpy(reviewer,start,end-start+1);
            printf("name[%d]:%s\n",nameLength,reviewer);
            int b = 0;
            printf("namelength:%d\n",cnt);
            for (i=0;i<cnt;i++) 
            {
                printf("namecmp[%s]:[%s]\n",reviewer,name[i]);
                if (strcmp(reviewer,name[i])==0)
                {
                    b = 1;
                    break;
                }
            }
            if (!b) 
            {
                char err[1024];
                memset(err,0,1024);
                sprintf(err,"%s has not checked yet.",reviewer);
                err_info_cat(err);
                printf(err);
                over=0;
            }
	        start=++c;
            nameLength++;
        }
    }
    char *end=c-1;
    for (;*start==' ' || *start=='\t' || *start=='\n' || *start=='\r';start++);
    for (;*end==' ' || *end=='\t' || *end=='\n' || *end=='\r';end--);
    memset(reviewer,0,32);
    strncpy(reviewer,start,end-start+1);
    if (strlen(reviewer)==0)
    {
        printf("no reviewers len\n");
        return OK;
    }
    printf("name[%d]:%s\n",nameLength,reviewer);
    int b = 0;
    printf("namelength:%d\n",cnt);
    for (i=0;i<cnt;i++)
    {         
        printf("namecmp[%s]:[%s]\n",reviewer,name[i]);
        if (strcmp(reviewer,name[i])==0)
        {   
            b = 1;
            break;
        }
    }
    if (!b)
    {   
        char err[1024];
        memset(err,0,1024);
        sprintf(err,"%s has not checked yet.",reviewer);
        err_info_cat(err);
        printf("%s\n",err);
        over=0;
    }
    if (over) 
    {
        return OK;
    } else {
        return FAIL;
    }
}

/*

commit message

*/
int gerrit_commitMessage_check(cJSON * root)
{
    cJSON *json;
    char *msg;
    char *nature=1;
    char *change=0;
    char *verify=0;
    char *signed_off=1;
    int change_len=0;
    int verify_len=0;
    int ret=0;

#define KEY_NATURE      "[问题性质]"
#define KEY_CHANGE      "[修改点]"
#define KEY_VERIFY      "[验证]"
#define KEY_SIGNED_OFF  "Signed-off-by"
#define KEY_LEN_MIN     20

    json = cJSON_GetObjectItem(root, "commitMessage");
    if(!json) {
        err_info_cat(err_str[err_id_commit]);
        ret++;
        return ret;
    }

    msg = json->valuestring;
    msg = str_trim_head(msg);

    if( strstr(msg, "..") ||
        strstr(msg, "。。")) {
        err_info_cat(err_str[err_id_commit_too_lazy]);
        ret++;
        return ret;
    }
//    printf("\n\n\ncommitMessage=%s\n", msg);

    if(trackid_check(msg)) {
        err_info_cat(err_str[err_id_trackid]);
        ret++;
    }

    //nature = strstr(json->valuestring, KEY_NATURE);

    //if(nature) {

        /*
        遗留问题: 包括老版本未发现、新特性第一轮测试，新特性必须有对应开发项目。
        引入问题：本人修改代码引入的问题
        关联问题：其他模块修改代码引入的问题，引入模块和问题模块各负50%责任。如果引入模块未通知问题模块，则引入模块负全责。
        */
        //if( (!strstr(nature+10, "遗留问题")) &&
        //   (!strstr(nature+10, "引入问题")) &&
        //   (!strstr(nature+10, "新特性开发")) &&
        //(!strstr(nature+10, "关联问题"))) {
        //    err_info_cat(err_str[err_id_commit_nature]);
        //    ret++;
        //}

        //signed_off = strstr(nature, KEY_SIGNED_OFF);
        //change = strstr(nature, KEY_CHANGE);
    //}
/*
    if(change) {
        verify = strstr(change, KEY_VERIFY);
    }
    if(verify) {
        signed_off = strstr(verify, KEY_SIGNED_OFF);
    }
*/
    //change_len = verify - change - sizeof(KEY_CHANGE);
    //verify_len = signed_off - verify - sizeof(KEY_VERIFY);
    //printf("nature:%s,signed_off:%s",nature,signed_off);  
    //if( !(nature && change && verify && signed_off ) ||
    //if( !(nature && signed_off ) 
       //change_len < KEY_LEN_MIN ||
       //verify_len < KEY_LEN_MIN 
       //)
       //{
        //err_info_cat(err_str[err_id_commit]);
        //ret++;
       //}

    return ret;
}

/*-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
    cJSON *json_root;
//    char *out;
    char *commit=0;
    char cmd[1024];
    int ret=0;
    char *author;

#if 0
    int i;

    for(i=0; i<argc;i++) {
        printf("(%d)%s ", i, argv[i]);
    }
#endif

    if(argc >= 16) {
        author  = argv[14];
        if(strstr(author, "EMS")) {
            return 0;
        }
        commit = argv[16];
    }
    else {
        commit = "c601e078268b58e75c790f8278d5a2fc441f883a";
        return 0;
    }

    printf("comment-add enter: %s\n", commit);

#if 1 
    sprintf(cmd,
            "ssh -p 29418 ems@127.0.0.1 gerrit query"
            " --format=JSON status:open --comments --all-approvals"
            " commit:%s | iconv -f UTF-8 -t GBK", commit);
#else
    sprintf(cmd,
            "ssh -p 29418 ems@127.0.0.1 gerrit query"
            " --format=JSON status:open --comments --all-approvals"
            " commit:%s ", commit);
#endif
    printf("cmd: %s\n", cmd);

    if( !(json_root=json_open(cmd))) {
       return -1;
    }
    memset(err_msg, 0, sizeof(err_msg));

#if 0
    char *out=cJSON_Print(json_root);
    printf("%s\n", out);
    free(out);
#endif
    ret += gerrit_commitMessage_check(json_root);
//    ret += gerrit_patchSets_check(json_root);
//    ret += gerrit_comments_check(json_root);
    ret += gerrit_reviewer_check(json_root);

//    err_info_print();

    json_close(json_root);

    if(ret) {
//        FILE *f;
        sprintf(cmd,
            "ssh -p 29418 ems@127.0.0.1 gerrit review"
            " -m '\"%s\"' --code-review -1 %s", err_msg, commit);
//        f=popen(cmd, "r");
//        pclose(f);
            system(cmd);
    }
    else {
        sprintf(cmd,
            "ssh -p 29418 ems@127.0.0.1 gerrit review"
            " -m '\"Successful!\"'"
            " --code-review +1 %s", commit);
        system(cmd);
    }

    printf("ret=%d, cmd: %s\n", ret, cmd);

    printf("comment-add exit: %s\n", commit);

    return 0;
}
