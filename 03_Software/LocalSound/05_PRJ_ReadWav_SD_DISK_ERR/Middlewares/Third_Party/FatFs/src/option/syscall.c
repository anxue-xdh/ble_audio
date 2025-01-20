/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2014                                                          */
/*------------------------------------------------------------------------*/

#include <stdlib.h> /* ANSI memory controls */
#include "../ff.h"

#include "semphr.h"

#if _FS_REENTRANT
/*-----------------------------------------------------------------------
 Create a Synchronization Object
------------------------------------------------------------------------
 This function is called in f_mount function to create a new
 synchronization object, such as semaphore and mutex. When a zero is
 returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_cre_syncobj(              /* TRUE:Function succeeded, FALSE:Could not create due to any error */
                   BYTE vol,     /* Corresponding logical drive being processed */
                   _SYNC_t *sobj /* Pointer to return the created sync object */
)
{
    //	/* Win32 */
    //	*sobj = CreateMutex(NULL, FALSE, NULL);
    //	return (int)(*sobj != INVALID_HANDLE_VALUE);

    /* uITRON */
    //	T_CSEM csem = {TA_TPRI,1,1};
    //	*sobj = acre_sem(&csem);
    //	return (int)(*sobj > 0);
    /* uC/OS-III */
    //	OS_ERR err;
    //
    //	OSMutexCreate ((OS_MUTEX*) sobj,
    //                     (CPU_CHAR*) "FATFS Mutex",
    //                     (OS_ERR*) &err);
    //	//return (int)(err == OS_ERR_NONE );
    //	return 1;

    /* FreeRTOS */
    *sobj = xSemaphoreCreateMutex();
    return (int)(*sobj != NULL);

    /* CMSIS-RTOS */
    //	*sobj = osMutexCreate(Mutex + vol);
    //	return (int)(*sobj != NULL);
}

/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to delete a synchronization
/  object that created with ff_cre_syncobj function. When a zero is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_del_syncobj(             /* TRUE:Function succeeded, FALSE:Could not delete due to any error */
                   _SYNC_t sobj /* Sync object tied to the logical drive to be deleted */
)
{
    //	/* Win32 */
    //	return (int)CloseHandle(sobj);

    /* uITRON */
    //	return (int)(del_sem(sobj) == E_OK);

    /* uC/OS-III */
    //	OS_ERR err;
    //	OSMutexDel(&sobj, OS_OPT_DEL_ALWAYS, &err);
    //	return (int)(err == OS_ERR_NONE );

    /* FreeRTOS */
    vSemaphoreDelete(sobj);
    return 1;

    /* CMSIS-RTOS */
    //	return (int)(osMutexDelete(sobj) == osOK);
}

/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a zero is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant(             /* TRUE:Got a grant to access the volume, FALSE:Could not get a grant */
                 _SYNC_t sobj /* Sync object to wait */
)
{
    //	/* Win32 */
    //	return (int)(WaitForSingleObject(sobj, FF_FS_TIMEOUT) == WAIT_OBJECT_0);

    /* uITRON */
    //	return (int)(wai_sem(sobj) == E_OK);

    /* uC/OS-III */
    //	OS_ERR err;

    //	OSMutexPend(&sobj,FF_FS_TIMEOUT, OS_OPT_PEND_BLOCKING,0, &err);
    //	return (int)(err == OS_ERR_NONE );

    /* FreeRTOS */
    return (int)(xSemaphoreTake(sobj, _FS_TIMEOUT) == pdTRUE);

    /* CMSIS-RTOS */
    //	return (int)(osMutexWait(sobj, FF_FS_TIMEOUT) == osOK);
}

/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
 */

void ff_rel_grant(
    _SYNC_t sobj /* Sync object to be signaled */
)
{
    //	/* Win32 */
    //	ReleaseMutex(sobj);

    /* uITRON */
    //	sig_sem(sobj);

    /* uC/OS-III */
    //	OS_ERR err;
    // OSMutexPost ((OS_MUTEX*) &sobj,
    //                   (OS_OPT)     OS_OPT_POST_NONE,					//2??¡äDD¨¬?¨ºa2¨´¡Á¡Â¡ê??¡ä2????1¦Ì¡Â?¨¨
    //                   (OS_ERR*)    &err);
    /* FreeRTOS */
    xSemaphoreGive(sobj);

    /* CMSIS-RTOS */
    //	osMutexRelease(sobj);
}
#endif

#if _USE_LFN == 3 /* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
 */
extern void *fatfs_malloc(UINT size);
extern void fatfs_free(void *ptr);

void *ff_memalloc(           /* Returns pointer to the allocated memory block */
                  UINT msize /* Number of bytes to allocate */
)
{
    return malloc(msize); /* Allocate a new memory block with POSIX API */
    // return fatfs_malloc(msize); /* Allocate a new memory block with POSIX API */
}

/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree(
    void *mblock /* Pointer to the memory block to free */
)
{
    free(mblock); /* Discard the memory block with POSIX API */
    // fatfs_free(mblock); /* Free the memory block with POSIX API */
}

#endif
