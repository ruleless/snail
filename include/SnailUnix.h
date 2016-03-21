#ifndef __SNAILUNIX_H__
#define __SNAILUNIX_H__

#include "platform.h"

#if PLATFORM == PLATFORM_UNIX

/* 输出错误信息并退出
 * @param msg: 错误信息
 * @param bErrno: 是否打印errno错误消息
 */
void errSys(const char *msg, bool bErrno = true);

void errQuit(const char *msg, bool bErrno = true);

/* 给指定区域加读锁(不阻塞)
 */
#define regReadRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLK, F_RDLCK, offset, whence, len)
/* 给指定区域加读锁(阻塞)
 */
#define regReadWRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLKW, F_RDLCK, offset, whence, len)
/* 给指定区域加写锁(不阻塞)
 */
#define regWriteRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLK, F_WRLCK, offset, whence, len)
/* 给指定区域加写锁(阻塞)
 */
#define regWriteWRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLKW, offset, whence, len)

/* 注册记录锁
 */
int regRecLock(int fd, int cmd, int type, off_t offset, int whence, off_t len);

/* 遍历目录
 */
typedef bool (*FileHandler)(const char *pathname);
int traverseDir(const char *pathname, FileHandler handler);

#endif

#endif // __SNAILUNIX_H__