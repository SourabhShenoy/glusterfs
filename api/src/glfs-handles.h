/*
  Copyright (c) 2013 Red Hat, Inc. <http://www.redhat.com>
  This file is part of GlusterFS.

  This file is licensed to you under your choice of the GNU Lesser
  General Public License, version 3 or any later version (LGPLv3 or
  later), or the GNU General Public License, version 2 (GPLv2), in all
  cases as published by the Free Software Foundation.
*/

#ifndef _GLFS_HANDLES_H
#define _GLFS_HANDLES_H

#include "glfs.h"

/* GLFS OBJECT BASED OPERATIONS
 *
 * The following APIs are introduced to provide an API framework that can work
 * with gluster objects (files and directories), instead of absolute paths.
 *
 * The following API set can be related to the POSIX *at interfaces (like
 * openat (2)). The intention of these APIs is to be able to operate based
 * on parent object and looking up or creating child objects within, OR to be
 * used on the actual object thus looked up or created, and retrieve information
 * regarding the same.
 *
 * The APIs also provide for generating an opaque invariant handle to the
 * object, that can later be used to lookup the object, instead of the regular
 * glfs_h_* variants. The APIs that provide this behaviour are,
 * glfs_h_extract_handle and glfs_h_create_from_handle.
 *
 * The object handles can be transitioned to fd based operations as supported
 * by glfs.h calls, using the glfs_h_open call. This provides a way to move
 * from objects to fd's akin to moving from path to fd for required operations.
 *
 * NOTE: The opaque invariant handle is the GFID of the object in reality, but
 * maintained as an opaque data value, for potential internal changes to the
 * same without impacting the caller.
 *
 * NOTE: Currently looking up an object can create multiple object handles to
 * the same, i.e distinct glfs_object *. Hence each such looked up or received
 * handle from other calls, would need to be closed. In the future, for a given
 * object these pointers would be the same, and an ease of use API to forget all
 * instances of this bject would be provided (instead of a per lookup close).
 * This should not change the APIs in their current form.
 *
 */

/* Values for valid falgs to be used when using XXXsetattr, to set multiple
 attribute values passed via the related stat structure.
 */
#define GFAPI_SET_ATTR_MODE  0x1
#define GFAPI_SET_ATTR_UID   0x2
#define GFAPI_SET_ATTR_GID   0x4
#define GFAPI_SET_ATTR_SIZE  0x8
#define GFAPI_SET_ATTR_ATIME 0x10
#define GFAPI_SET_ATTR_MTIME 0x20

/* Handle length for object handles returned from glfs_h_extract_handle or
 * glfs_h_create_from_handle */
#define GFAPI_HANDLE_LENGTH 16

/* These flags should be in sync to the ones defined in upcall.h */
#define UP_NLINK   0x00000001   /* update nlink */
#define UP_MODE    0x00000002   /* update mode and ctime */
#define UP_OWN     0x00000004   /* update mode,uid,gid and ctime */
#define UP_SIZE    0x00000008   /* update fsize */
#define UP_TIMES   0x00000010   /* update all times */
#define UP_ATIME   0x00000020   /* update atime only */
#define UP_PERM    0x00000040   /* update fields needed for
                                   permission checking */
#define UP_RENAME  0x00000080   /* this is a rename op -
                                   delete the cache entry */

#define INODE_UPDATE_FLAGS (UP_NLINK | UP_MODE | \
                            UP_OWN | UP_SIZE | \
                            UP_TIMES | UP_ATIME)

/* Portability non glibc c++ build systems */
#ifndef __THROW
# if defined __cplusplus
#  define __THROW       throw ()
# else
#  define __THROW
# endif
#endif

__BEGIN_DECLS

/*
 * Notes:
 *
 * The file object handle. One per looked up, created file/directory
 *
 * This had been introduced to facilitate gfid/inode based gfapi
 * - a requirement introduced by nfs-ganesha
 */
struct glfs_object;
typedef struct glfs_object glfs_object_t;

/*
 * Applications (currently NFS-Ganesha) can make use of this
 * structure to read upcall notifications sent by server.
 *
 * On success, applications need to check for 'object' to decide
 * if any upcall event is received.
 *
 * After processing the event, they need to free "object"
 * using glfs_h_close(..).
 */
struct callback_arg {
        struct glfs             *fs; /* glfs object */
        int                     reason;  /* Upcall event type */
        struct glfs_object      *object; /* Object which need to be acted upon */
        int                     flags; /* Cache UPDATE/INVALIDATE flags */
        struct stat             buf; /* Latest stat of this entry */
        unsigned int            expire_time_attr; /* the amount of time for which
                                                   * the application need to cache
                                                   * this entry
                                                   */
};

/* reason list in callback_arg */
enum callback_type {
        CBK_EVENT_NULL,
        INODE_INVALIDATE,
        INODE_UPDATE,
};

/* Handle based operations */
/* Operations that generate handles */
struct glfs_object *glfs_h_lookupat (struct glfs *fs,
				     struct glfs_object *parent,
				     const char *path,
                                     struct stat *stat) __THROW
        GFAPI_PUBLIC(glfs_h_lookupat, 3.4.0);

struct glfs_object *glfs_h_creat (struct glfs *fs, struct glfs_object *parent,
				  const char *path, int flags, mode_t mode,
				  struct stat *sb) __THROW
        GFAPI_PUBLIC(glfs_h_create, 3.4.0);

struct glfs_object *glfs_h_mkdir (struct glfs *fs, struct glfs_object *parent,
				  const char *path, mode_t flags,
				  struct stat *sb) __THROW
        GFAPI_PUBLIC(glfs_h_mkdir, 3.4.0);

struct glfs_object *glfs_h_mknod (struct glfs *fs, struct glfs_object *parent,
				  const char *path, mode_t mode, dev_t dev,
				  struct stat *sb) __THROW
        GFAPI_PUBLIC(glfs_h_mknod, 3.4.0);

struct glfs_object *glfs_h_symlink (struct glfs *fs, struct glfs_object *parent,
				    const char *name, const char *data,
				    struct stat *stat) __THROW
        GFAPI_PUBLIC(glfs_h_symlink, 3.4.0);

/* Operations on the actual objects */
int glfs_h_unlink (struct glfs *fs, struct glfs_object *parent,
		   const char *path) __THROW
        GFAPI_PUBLIC(glfs_h_unlink, 3.4.0);

int glfs_h_close (struct glfs_object *object) __THROW
        GFAPI_PUBLIC(glfs_h_close, 3.4.0);

int glfs_caller_specific_init (void *uid_caller_key, void *gid_caller_key,
			       void *future) __THROW
        GFAPI_PUBLIC(glfs_h_lookupat, 3.5.0);

int glfs_h_truncate (struct glfs *fs, struct glfs_object *object,
                     off_t offset) __THROW
        GFAPI_PUBLIC(glfs_h_truncate, 3.4.0);

int glfs_h_stat(struct glfs *fs, struct glfs_object *object,
                struct stat *stat) __THROW
        GFAPI_PUBLIC(glfs_h_stat, 3.4.0);

int glfs_h_getattrs (struct glfs *fs, struct glfs_object *object,
		     struct stat *stat) __THROW
        GFAPI_PUBLIC(glfs_h_getattrs, 3.4.0);

int glfs_h_getxattrs (struct glfs *fs, struct glfs_object *object,
		      const char *name, void *value,
		      size_t size) __THROW
        GFAPI_PUBLIC(glfs_h_lookupat, 3.5.1);

int glfs_h_setattrs (struct glfs *fs, struct glfs_object *object,
		     struct stat *sb, int valid) __THROW
        GFAPI_PUBLIC(glfs_h_setattrs, 3.4.0);

int glfs_h_setxattrs (struct glfs *fs, struct glfs_object *object,
		      const char *name, const void *value,
		      size_t size, int flags) __THROW
        GFAPI_PUBLIC(glfs_h_setxattrs, 3.5.0);

int glfs_h_readlink (struct glfs *fs, struct glfs_object *object, char *buf,
		     size_t bufsiz) __THROW
        GFAPI_PUBLIC(glfs_h_readlink, 3.4.0);

int glfs_h_link (struct glfs *fs, struct glfs_object *linktgt,
		 struct glfs_object *parent, const char *name) __THROW
        GFAPI_PUBLIC(glfs_h_link, 3.4.0);

int glfs_h_rename (struct glfs *fs, struct glfs_object *olddir,
		   const char *oldname, struct glfs_object *newdir,
		   const char *newname) __THROW
        GFAPI_PUBLIC(glfs_h_rename, 3.4.0);

int glfs_h_removexattrs (struct glfs *fs, struct glfs_object *object,
			 const char *name) __THROW
        GFAPI_PUBLIC(glfs_h_removexattrs, 3.5.1);

/* Operations enabling opaque invariant handle to object transitions */
ssize_t glfs_h_extract_handle (struct glfs_object *object,
			       unsigned char *handle, int len) __THROW
        GFAPI_PUBLIC(glfs_h_extract_handle, 3.4.0);

struct glfs_object *glfs_h_create_from_handle (struct glfs *fs,
					       unsigned char *handle, int len,
					       struct stat *stat) __THROW
        GFAPI_PUBLIC(glfs_h_create_from_handle, 3.4.0);

/* Operations enabling object handles to fd transitions */
struct glfs_fd *glfs_h_opendir (struct glfs *fs,
                                struct glfs_object *object) __THROW
        GFAPI_PUBLIC(glfs_h_opendir, 3.4.0);

struct glfs_fd *glfs_h_open (struct glfs *fs, struct glfs_object *object,
			     int flags) __THROW
        GFAPI_PUBLIC(glfs_h_open, 3.4.0);

int
glfs_h_access (struct glfs *fs, struct glfs_object *object, int mask) __THROW
        GFAPI_PUBLIC(glfs_h_access, 3.6.0);

/*
  SYNOPSIS

  glfs_h_poll_upcall: Poll for upcall events given a 'glfs' object.

  DESCRIPTION

  This API is used to poll for upcall events stored in the
  upcall list. Current users of this API is NFS-Ganesha.
  Incase of any event received, it will be mapped appropriately
  into 'callback_arg' along with the handle('glfs_object') to be
  passed to NFS-Ganesha.

  In case of success, applications need to check the value of
  cbk->handle to be NON NULL before processing the upcall
  events.

  PARAMETERS

  @fs: glfs object to poll the upcall events for
  @cbk: Structure to store upcall events as desired by the application.
        Application is responsible for allocating and passing the
        references of all the pointers of this structure except for
        "handle". In case of any events received, it needs to free
        "handle"

  RETURN VALUES

  0   : Success.
  -1  : Error condition, mostly due to out of memory.

*/

int
glfs_h_poll_upcall (struct glfs *fs, struct callback_arg *cbk) __THROW
        GFAPI_PUBLIC(glfs_h_poll_upcall, 3.7.0);

int
glfs_h_acl_set (struct glfs *fs, struct glfs_object *object,
                const acl_type_t type, const acl_t acl) __THROW;
        GFAPI_PUBLIC(glfs_h_acl_set, 3.7.0);

acl_t
glfs_h_acl_get (struct glfs *fs, struct glfs_object *object,
                const acl_type_t type) __THROW;
        GFAPI_PUBLIC(glfs_h_acl_get, 3.7.0);

__END_DECLS

#endif /* !_GLFS_HANDLES_H */
