#ifndef __COMMON_H__
#define __COMMON_H__

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a, b) ((a)<(b)?(a):(b))
#endif

#define PRINT_INTVAL(val) printf("%s:%d\n", #val, val)

#ifndef NAME_PATH
#define NAME_PATH 255
#endif
#ifndef NAME_MAX
#define NAME_MAX 255
#endif

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

#if defined (__i386__)
inline uint64_t getTimeStamp()
{
	uint64_t x;
	__asm__ volatile("rdtsc":"=A"(x));
	return x;
}
#elif defined (__x86_64__)
inline uint64_t getTimeStamp()
{
	unsigned hi,lo;
	__asm__ volatile("rdtsc":"=a"(lo),"=d"(hi));
	return ((uint64_t)lo)|(((uint64_t)hi)<<32);
}
#elif defined (__WIN32__) || defined(_WIN32) || defined(WIN32)
inline uint64_t getTimeStamp()
{
    __asm
    {
        _emit 0x0F;
        _emit 0x31;
    }
}
#endif

#endif
