/*
 *      webu.h
 *
 *      Include file for webu.c
 *
 *      This software is distributed under the GNU Public License Version 2
 *      See also the file 'COPYING'.
 *
 */
#ifndef _INCLUDE_WEBU_H_
#define _INCLUDE_WEBU_H_


/* Some defines of lengths for our buffers */
#define WEBUI_LEN_PARM 512          /* Parameters specified */
#define WEBUI_LEN_URLI 512          /* Maximum URL permitted */
#define WEBUI_LEN_RESP 1024         /* Initial response size */
#define WEBUI_MHD_OPTS 10           /* Maximum number of options permitted for MHD */
#define WEBUI_LEN_LNK  15           /* Maximum length for chars in strminfo */

struct webui_ctx {
    char *url;                   /* The URL sent from the client */
    char *uri_camid;            /* Parsed thread number from the url*/
    char *uri_cmd1;              /* Parsed command(action) from the url*/
    char *uri_cmd2;              /* Parsed command (set) from the url*/
    char *uri_parm1;             /* Parameter 1 for the command */
    char *uri_value1;            /* The value for parameter 1*/
    char *uri_parm2;             /* Parameter 2 for the command */
    char *uri_value2;            /* The value for parameter 2*/

    char *hostname;              /* Host name provided from header content*/
    char  hostproto[6];          /* Protocol for host http or https */
    char *clientip;              /* IP of the connecting client */
    char *auth_denied;          /* Denied access response to user*/
    char *auth_opaque;          /* Opaque string for digest authentication*/
    char *auth_realm;           /* Realm string for digest authentication*/
    char *auth_user;            /* Parsed user from config authentication string*/
    char *auth_pass;            /* Parsed password from config authentication string*/
    int  authenticated;         /* Boolean for whether authentication has been passed */

    int   cam_count;             /* Count of the number of cameras*/
    int   cam_threads;           /* Count of the number of camera threads running*/
    char *lang;                  /* Two character abbreviation for locale language*/
    char *lang_full;             /* Five character abbreviation for language-country*/
    int   thread_nbr;            /* Thread number provided from the uri */

    char            *resp_page;         /* The response that will be sent */
    size_t           resp_size;         /* The allocated size of the response */
    size_t           resp_used;         /* The amount of the response page used */
    int              mhd_first;         /* Boolean for whether it is the first connection*/
    unsigned char   *stream_img;        /* Copy of the image to stream from cnt*/
    unsigned char   *stream_imgsub;     /* Substream image */
    size_t           stream_img_size;   /* Size of the image provided from cnt */
    int              valid_subsize;     /* Boolean for whether substream dimensions are valid*/

    struct MHD_Connection  *connection;
    struct context        **cntlst;
    struct context         *cnt;

};

void webu_start(struct context **cnt);
void webu_stop(struct context **cnt);
void webu_process_action(struct webui_ctx *webui);
int webu_process_config(struct webui_ctx *webui);
int webu_process_track(struct webui_ctx *webui);
void webu_write(struct webui_ctx *webui, const char *buf);

#endif
