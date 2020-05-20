/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2019 NXP
 */

/**
 * @addtogroup	dxgrOAL
 * @{
 * 
 * @defgroup    dxgr_OAL_JOB JOB
 * @brief		Deferred job abstraction
 * @details		TODO     
 * 				
 * 
 * @addtogroup  dxgr_OAL_JOB
 * @{
 * 
 * @file		oal_job.h
 * @brief		The oal_job module header file.
 * @details		This file contains generic deferred job management-related API.
 *
 */

#ifndef PUBLIC_OAL_JOB_H_
#define PUBLIC_OAL_JOB_H_

typedef struct __oal_job_tag oal_job_t;
typedef void (* oal_job_func)(void *arg);

/**
 * @brief	Priority enumeration type
 */
typedef enum
{
	OAL_PRIO_LOW,
	OAL_PRIO_NORMAL,
	OAL_PRIO_HIGH,
	OAL_PRIO_TOP
} oal_prio_t;

/**
 * @brief		Create new job
 * @param[in]	func Function to be executed within the job
 * @param[in]	arg Function argument
 * @param[in]	name The job name in string form
 * @param[in]	prio Job priority
 * @return		New job instance or NULL if failed
 */
oal_job_t *oal_job_create(oal_job_func func, void *arg, const char_t *name, oal_prio_t prio);

/**
 * @brief		Destroy job
 * @details		Function will wait until job is done and then dispose the instance
 * @param[in]	job The job instance
 * @return		EOK if success, error code otherwise
 */
errno_t oal_job_destroy(oal_job_t *job);

/**
 * @brief		Trigger job execution
 * @details		Schedule the job. Can be called multiple times to enqueue multiple
 * 				triggers. Is a non-blocking call.
 * @param[in]	job The job instance
 * @return		EOK if success, error code otherwise
 */
errno_t oal_job_run(oal_job_t *job);

/**
 * @brief		Wait until job is done
 * @param[in]	job The job instance
 * @return		EOK if success, error code otherwise
 */
errno_t oal_job_drain(oal_job_t *job);

#endif /* PUBLIC_OAL_JOB_H_ */

/** @}*/
/** @}*/
