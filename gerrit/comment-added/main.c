/*
1. �ύ�����˵����������ϸ�ʽ��
��ʽ���£�
-----------------------------------------------------------
[���ⵥ��] ��������
[�޸ĵ�]
1. XXXXXXXXX
2. XXXXXXX
[��֤]
1. XXXXX
2. XXXXX

Signed-off-by: XXXXX <XXXX@dvt.dvt.com>
-----------------------------------------------------------

���У������������޸ĵ����֤����������������10�����֡�


2. �����������ߣ������ύ�����ڣ���������2�ˡ�

3. ÿ�������֣����������������������������10�����֡�

4. ÿ�������¼��������עһ��Դ�����ļ���������������10�����֡�

5. �ڴ�����˹����з��ֵ��������⣬���ڴ����Խ�硢й¶������Խ�硢�ƻ����й��ܵȵȣ����ⷢ��������������Ϊ����Ŀ���ڵ���ľ�񽱡�

6. ��С��ͳ����Ч����������˾���Ч���������������С�飬���»���˾�С������50Ԫ������Ϊ�鳤�������������������к���׿��ˡ�
�����µ׹�ʾһ�죬 ������ɼ��, ��Ū��������ȡ���ʸ�

ע��
1��4��ǿ���Ա�׼�������Ͻ����������ܾ��ύ��

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cJSON.h"
#include "tools.h"

#define VALID_COMMENT_LEN 20

char *err_str[]= {
    "Bug title wrong; ", //���ⵥ��ʽ����; ",
    "Commit message format wrong; ", //�ύ��Ϣ��ʽ����; ",
    "Approvaled by less than 3 guys; ", //" ���������2��; ",
    "Review comments less than 20 bytes; ", //"����������10�����ֻ�20���ַ�; ",
    "File comments less than 20 bytes, add a long comment pls. ", //"�ļ���ע�������10�����ֻ�20���ַ�: ",
    "File comments absent or too short; ", //ȱ�ļ���ע; ",
    "Review record duplicate; ", //"ͬһ�����ж�������¼",
    "commit msg too lazy; ", //"����͵������㲻����",
    "commit message bug nature absent; ", //ȱ[��������]
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
    printf("%s", err_msg);
}

static char data[128*1024];
/*-----------------------------------------------------------------------------*/
/* Read a file, parse, render back, etc. */
cJSON * json_open(char *cmd)
{
    FILE *f;
    cJSON * json;

//    printf("cmd: %s\n", cmd);

	if(!(f=popen(cmd,"rb"))) {
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

        /* gerrit�ʺ����Զ�����Ӧ���ų� */
        if(strcmp(username, "gerrit")) {
            valid_cnt++;
        }
    }

    if(valid_cnt < 3) {
        return FAIL;
    }

    return OK;
}

/* ����ļ���ע��ֻҪ��һ������20�־Ϳ��� */
int gerrit_patchSet_comments_check(cJSON *patchSet)
{
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
           (!strstr(msg->valuestring, "��������")) ) {
            ret = 0;
        }
    }

    return ret;
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

        if( (!patchSet_comment_ok) && (!gerrit_patchSet_comments_check(patchSet))) {
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
        (strstr(msg, "����"))) {
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

/* ֻҪ��һ�������������20�ַ��͹��� */
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

        if(strcmp(username->valuestring, "gerrit") &&
                   !msg_valid(msg->valuestring)) {
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
������ⵥ��
[OLT-100] XXXXXXXXXX

*/

static char *project_names[]= {
    "OLT",
    "ONU",
    "CCMTS",
    "EMS",
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

    /* ��⿪ʼ��[ */
    if (*msg != '['){
        return FAIL;
    }
    msg++;

    /* �ҵ�] */
    if( !(p=strstr(msg, "]"))) {
       return FAIL;
    }

    /* ���ⵥ���� */
    if( !(tail=strstr(p, "\n"))) {
       return FAIL;
    }
    /* ���ⵥ������������10�����֣���20���ַ� */
    if(tail - p < VALID_COMMENT_LEN) {
        return FAIL;
    }

    /* ���Ƶ��� */
    memcpy(trackid, msg, p - msg);

    /* �ҵ� - */
    if( !(p=strstr(trackid, "-") )){
       return FAIL;
    }

    /* �ָ���������� */
    *p = '\0';
    p++;

    /* ��鹤���� */
    if(project_match(trackid)) {
        return FAIL;
    }

    /* ��鵥���Ƿ����� */
    if(is_number(p)) {
        return FAIL;
    }

    if(sscanf(p, "%d", &trackno) <= 0) {
        return FAIL;
    }

    /* ���Ƶ���ҪС��40000�������Թ��Ŵ��� */
    if(trackno >= 40000){
        return FAIL;
    }

    return OK;
}

/*

    ���commit message

*/
int gerrit_commitMessage_check(cJSON * root)
{
    cJSON *json;
    char *msg;
    char *nature=0;
    char *change=0;
    char *verify=0;
    char *signed_off=0;
    int change_len=0;
    int verify_len=0;
    int ret=0;

#define KEY_NATURE      "[��������]"
#define KEY_CHANGE      "[�޸ĵ�]"
#define KEY_VERIFY      "[��֤]"
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
        strstr(msg, "����")) {
        err_info_cat(err_str[err_id_commit_too_lazy]);
        ret++;
        return ret;
    }
//    printf("\n\n\ncommitMessage=%s\n", msg);

    if(trackid_check(msg)) {
        err_info_cat(err_str[err_id_trackid]);
        ret++;
    }

    nature = strstr(json->valuestring, KEY_NATURE);

    if(nature) {

        /*
        ��������: �����ϰ汾δ���֡������Ե�һ�ֲ��ԣ������Ա����ж�Ӧ������Ŀ��
        �������⣺�����޸Ĵ������������
        �������⣺����ģ���޸Ĵ�����������⣬����ģ�������ģ�����50%���Ρ��������ģ��δ֪ͨ����ģ�飬������ģ�鸺ȫ��
        */
        if( (!strstr(nature+10, "��������")) &&
           (!strstr(nature+10, "��������")) &&
           (!strstr(nature+10, "�����Կ���")) &&
        (!strstr(nature+10, "��������"))) {
            err_info_cat(err_str[err_id_commit_nature]);
            ret++;
        }

        change = strstr(nature, KEY_CHANGE);
    }

    if(change) {
        verify = strstr(change, KEY_VERIFY);
    }
    if(verify) {
        signed_off = strstr(verify, KEY_SIGNED_OFF);
    }

    change_len = verify - change - sizeof(KEY_CHANGE);
    verify_len = signed_off - verify - sizeof(KEY_VERIFY);

    if( !(nature && change && verify && signed_off ) ||
       change_len < KEY_LEN_MIN ||
       verify_len < KEY_LEN_MIN ) {
        err_info_cat(err_str[err_id_commit]);
        ret++;
       }

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
        if(strstr(author, "gerrit")) {
            return 0;
        }
        commit = argv[16];
    }
    else {
        commit = "c601e078268b58e75c790f8278d5a2fc441f883a";
//        return 0;
    }

    printf("comment-add enter: %s\n", commit);

#if 1
    sprintf(cmd,
            "ssh -p 29418 gerrit@127.0.0.1 gerrit query"
            " --format=JSON status:open --comments --all-approvals"
            " commit:%s | iconv -f UTF-8 -t GBK", commit);
#else
    sprintf(cmd,
            "ssh -p 29418 gerrit@127.0.0.1 gerrit query"
            " --format=JSON status:open --comments --all-approvals"
            " commit:%s ", commit);
#endif
    if( !(json_root=json_open(cmd))) {
       return -1;
    }

    memset(err_msg, 0, sizeof(err_msg));

#if 0
    out=cJSON_Print(json_root);
    printf("%s\n", out);
    free(out);
#endif // 0

    ret += gerrit_commitMessage_check(json_root);
    ret += gerrit_patchSets_check(json_root);
    ret += gerrit_comments_check(json_root);

//    err_info_print();

    json_close(json_root);

    if(ret) {
//        FILE *f;
        sprintf(cmd,
            "ssh -p 29418 gerrit@127.0.0.1 gerrit review"
            " -m '\"%s\"' --code-review -1 %s", err_msg, commit);
//        f=popen(cmd, "r");
//        pclose(f);
            system(cmd);
    }
    else {
        sprintf(cmd,
            "ssh -p 29418 gerrit@127.0.0.1 gerrit review"
            " -m '\"Successful!\"'"
            " --code-review +1 %s", commit);
        system(cmd);
    }

    printf("ret=%d, cmd: %s\n", ret, cmd);

    printf("comment-add exit: %s\n", commit);

    return 0;
}
