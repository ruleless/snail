#ifndef __SNAILUNIX_H__
#define __SNAILUNIX_H__

#include "platform.h"

#if PLATFORM == PLATFORM_UNIX

/* ���������Ϣ���˳�
 * @param msg: ������Ϣ
 * @param bErrno: �Ƿ��ӡerrno������Ϣ
 */
void errSys(const char *msg, bool bErrno = true);

void errQuit(const char *msg, bool bErrno = true);

/* ��ָ������Ӷ���(������)
 */
#define regReadRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLK, F_RDLCK, offset, whence, len)
/* ��ָ������Ӷ���(����)
 */
#define regReadWRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLKW, F_RDLCK, offset, whence, len)
/* ��ָ�������д��(������)
 */
#define regWriteRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLK, F_WRLCK, offset, whence, len)
/* ��ָ�������д��(����)
 */
#define regWriteWRecLock(fd, offset, whence, len) \
	regRecLock(fd, F_SETLKW, offset, whence, len)

/* ע���¼��
 */
int regRecLock(int fd, int cmd, int type, off_t offset, int whence, off_t len);

/* ����Ŀ¼
 */
typedef bool (*FileHandler)(const char *pathname);
int traverseDir(const char *pathname, FileHandler handler);

#endif

#endif // __SNAILUNIX_H__