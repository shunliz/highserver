#ifndef LOCKER_H
#define LOCKER_H

#ifdef __cplusplus
extern "C" {
#endif

enum Ret
{
	RET_INVALID_PARAMS,
	RET_OK,
	RET_FAIL
};


struct _Locker;
typedef struct _Locker Locker;
typedef enum Ret  (*LockerLockFunc)(Locker* thiz);
typedef enum Ret  (*LockerUnlockFunc)(Locker* thiz);
typedef enum Ret (*LockerDestroyFunc)(Locker* thiz);
struct _Locker{    
	LockerLockFunc    lock;
	LockerUnlockFunc  unlock;    
	LockerDestroyFunc destroy;    
	char priv[0];
};

#ifdef __cplusplus
}
#endif

#endif
