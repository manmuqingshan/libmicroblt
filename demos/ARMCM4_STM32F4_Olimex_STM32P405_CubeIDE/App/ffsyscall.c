/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2014                                                          */
/*------------------------------------------------------------------------*/


#include "ff.h"
#include "microtbx.h"


#if _FS_REENTRANT
/*------------------------------------------------------------------------*/
/* Create a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object, such as semaphore and mutex. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* 1:Function succeeded, 0:Could not create the sync object */
	BYTE vol,			/* Corresponding volume (logical drive number) */
	_SYNC_t *sobj		/* Pointer to return the created sync object */
)
{
	int ret;

	*sobj = xSemaphoreCreateMutex();	/* FreeRTOS */
	ret = (int)(*sobj != NULL);

	return ret;
}



/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* 1:Function succeeded, 0:Could not delete due to any error */
	_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
	int ret;

  vSemaphoreDelete(sobj);		/* FreeRTOS */
	ret = 1;

	return ret;
}



/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* 1:Got a grant to access the volume, 0:Could not get a grant */
	_SYNC_t sobj	/* Sync object to wait */
)
{
	int ret;

	ret = (int)(xSemaphoreTake(sobj, _FS_TIMEOUT) == pdTRUE);	/* FreeRTOS */

	return ret;
}



/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	_SYNC_t sobj	/* Sync object to be signaled */
)
{
	xSemaphoreGive(sobj);	/* FreeRTOS */
}

#endif




#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block */
	UINT msize		/* Number of bytes to allocate */
)
{
  void * result;

  /* Attempt to allocate a block from a memory pool with the same size. If non-existant,
   * automatically create the memory pool. If no more free blocks are available in the
   * memory pool, automatically expand the memory pool by adding one more block.
   */
  result = TbxMemPoolAllocateAuto(msize);
  /* Assert the allocation result for debugging purposes. */
  TBX_ASSERT(result != NULL);

  /* Give the result back to the caller. */
  return result;
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
	void* mblock	/* Pointer to the memory block to free */
)
{
  /* Give the block back to the memory pool. */
  TbxMemPoolRelease(mblock);
}

#endif
