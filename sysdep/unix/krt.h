/*
 *	BIRD -- UNIX Kernel Route Syncer
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_H_
#define _BIRD_KRT_H_

struct config;
struct krt_config;
struct krt_proto;
struct kif_config;
struct kif_proto;

#include "lib/krt-scan.h"
#include "lib/krt-set.h"
#include "lib/krt-iface.h"

/* Flags stored in net->n.flags, rest are in nest/route.h */

#define KRF_VERDICT_MASK 0x0f
#define KRF_CREATE 0			/* Not seen in kernel table */
#define KRF_SEEN 1			/* Seen in kernel table during last scan */
#define KRF_UPDATE 2			/* Need to update this entry */
#define KRF_DELETE 3			/* Should be deleted */
#define KRF_IGNORE 4			/* To be ignored */

#define EA_KRT_PREFSRC EA_CODE(EAP_KRT, 0)
#define EA_KRT_REALM EA_CODE(EAP_KRT, 1)

/* Whenever we recognize our own routes, we allow learing of foreign routes */

#ifdef CONFIG_SELF_CONSCIOUS
#define KRT_ALLOW_LEARN
#endif

/* krt.c */

extern struct protocol proto_unix_kernel;

struct krt_config {
  struct proto_config c;
  struct krt_set_params set;
  struct krt_scan_params scan;
  int persist;			/* Keep routes when we exit */
  int scan_time;		/* How often we re-scan routes */
  int learn;			/* Learn routes from other sources */
  int devroutes;		/* Allow export of device routes */
};

struct krt_proto {
  struct proto p;
  struct krt_set_status set;
  struct krt_scan_status scan;
  struct krt_if_status iface;
#ifdef KRT_ALLOW_LEARN
  struct rtable krt_table;	/* Internal table of inherited routes */
#endif
  pool *krt_pool;		/* Pool used for common krt data */
  timer *scan_timer;
#ifdef CONFIG_ALL_TABLES_AT_ONCE
  node instance_node;		/* Node in krt instance list */
#endif
  int initialized;		/* First scan has already been finished */
};

extern struct proto_config *cf_krt;
extern pool *krt_pool;

#define KRT_CF ((struct krt_config *)p->p.cf)

#define KRT_TRACE(pr, fl, msg, args...) do {	\
  DBG("KRT: " msg "\n" , ## args);		\
  if (pr->p.debug & fl)				\
    { log(L_TRACE "%s: " msg, pr->p.name , ## args); } } while(0)

void krt_got_route(struct krt_proto *p, struct rte *e);
void krt_got_route_async(struct krt_proto *p, struct rte *e, int new);

/* Values for rte->u.krt_sync.src */
#define KRT_SRC_UNKNOWN	-1	/* Nobody knows */
#define KRT_SRC_BIRD	 0	/* Our route (not passed in async mode) */
#define KRT_SRC_REDIRECT 1	/* Redirect route, delete it */
#define KRT_SRC_ALIEN	 2	/* Route installed by someone else */
#define KRT_SRC_KERNEL	 3	/* Kernel routes, are ignored by krt syncer */

extern struct protocol proto_unix_iface;

struct kif_primary_item {
  node n;
  byte *pattern;
  ip_addr prefix;
  int pxlen;
};

struct kif_config {
  struct proto_config c;
  struct krt_if_params iface;
  int scan_time;		/* How often we re-scan interfaces */
  list primary;			/* Preferences for primary addresses */
};

struct kif_proto {
  struct proto p;
  struct krt_if_status iface;
};

extern struct proto_config *cf_kif;

#define KIF_CF ((struct kif_config *)p->p.cf)

/* krt-scan.c */

void krt_scan_preconfig(struct config *);
void krt_scan_postconfig(struct krt_config *);
void krt_scan_construct(struct krt_config *);
void krt_scan_start(struct krt_proto *, int);
void krt_scan_shutdown(struct krt_proto *, int);

void krt_scan_fire(struct krt_proto *);

/* krt-set.c */

void krt_set_construct(struct krt_config *);
void krt_set_start(struct krt_proto *, int);
void krt_set_shutdown(struct krt_proto *, int);

int krt_capable(rte *e);
void krt_set_notify(struct krt_proto *x, net *net, rte *new, rte *old);

/* krt-iface.c */

void krt_if_construct(struct kif_config *);
void krt_if_start(struct kif_proto *);
void krt_if_shutdown(struct kif_proto *);

void krt_if_scan(struct kif_proto *);
void krt_if_io_init(void);

#endif
