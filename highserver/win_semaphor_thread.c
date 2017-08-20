#include "windows.h"
#include "Locker.h"



typedef struct _PrivInfo{    
HANDLE semHandle;
}PrivInfo;


static enum Ret  locker_win_lock(Locker* thiz){
    PrivInfo* priv = (PrivInfo*)thiz->priv;
    int ret = WaitForSingleObject(priv->semHandle, INFINITE);
    return ret == 0 ? RET_OK : RET_FAIL;
}


static enum Ret  locker_win_unlock(Locker* thiz){
    PrivInfo* priv = (PrivInfo*)thiz->priv;
    int ret = ReleaseSemaphore(priv->semHandle, 1, NULL);
    return ret == 0 ? RET_OK : RET_FAIL;
}

static enum Ret  locker_win_destroy(Locker* thiz){
    PrivInfo* priv = (PrivInfo*)thiz->priv;
    int ret = CloseHandle (priv->semHandle) ;
    return ret == 0 ? RET_OK : RET_FAIL;
}


Locker* locker_win_create(void){    
	Locker* thiz = (Locker*)malloc(sizeof(Locker) + sizeof(PrivInfo));    
	if(thiz != NULL){
		PrivInfo* priv = (PrivInfo*)thiz->priv;
	        thiz->lock    = locker_win_lock;
	        thiz->unlock  = locker_win_unlock;
	        thiz->destroy = locker_win_destroy;
	        priv->semHandle = CreateSemaphore(NULL, 1, 256000, NULL);
                if( priv->semHandle == NULL)
                {
					return NULL;
				}
        }
        return thiz;
}


//single thread
//DList* dlist = dlist_create(NULL, NULL, NULL);
//multi thread
//DList* dlist = dlist_create(NULL, NULL, locker_pthread_create());
