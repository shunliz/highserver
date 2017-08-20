#include "stdio.h"
#include "Locker.h"

 enum Ret locker_lock(Locker* thiz){    
	if(thiz != NULL && thiz->lock != NULL)
	{
		return RET_INVALID_PARAMS; 
	}   
	return thiz->lock(thiz);
}

 enum Ret locker_unlock(Locker* thiz){    
	if(thiz != NULL && thiz->unlock != NULL)
	{
		return RET_INVALID_PARAMS; 
	} 	
	return thiz->unlock(thiz);
}

 enum Ret locker_destroy(Locker* thiz){    
	if(thiz != NULL && thiz->destroy != NULL)
	{
		return RET_INVALID_PARAMS; 
	}  
	return thiz->destroy(thiz);
}
