// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2020 NXP
 */

#include <stdbool.h>

#include "oal.h"
#include "oal_mm.h"
#include "oal_sync.h"
#include "hal.h"
#include "fifo.h"

#define is_power_of_2(n) ((n) && !((n) & ((n) - 1)))

__attribute__((hot)) errno_t
fifo_get_fill_level(fifo_t *const fifo, uint32_t *fill_level)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!fifo) || (!fill_level))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	*fill_level = (fifo->write - fifo->read);
	return EOK;
}

__attribute__((hot)) errno_t
fifo_get_free_space(fifo_t *const fifo, uint32_t *free_space)
{
	uint32_t ret;
	errno_t err;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!fifo) || (!free_space))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	err = fifo_get_fill_level(fifo, &ret);
	*free_space = fifo->depth - ret;

	return err;
}

__attribute__((cold)) fifo_t *
fifo_create(const uint32_t depth)
{
	fifo_t *fifo;

	if (!is_power_of_2(depth) || (depth > 0x7FFFFFFFU)) {
		return NULL;
	}

	fifo = (fifo_t *)oal_mm_malloc_contig_aligned_cache(
		sizeof(fifo_t), HAL_CACHE_LINE_SIZE);
	if (fifo) {
		fifo->read = 0U;
		fifo->write = 0U;
		fifo->depth = depth;
		fifo->depth_mask = depth - 1U;

		fifo->data = oal_mm_malloc_contig_aligned_cache(
			sizeof(void *) * depth, HAL_CACHE_LINE_SIZE);
		if (unlikely(!fifo->data)) {
			oal_mm_free(fifo);
			fifo = NULL;
		}
	}

	return fifo;
}

__attribute__((cold)) void
fifo_destroy(fifo_t *const fifo)
{
	if (fifo) {
		if (unlikely(fifo->data)) {
			oal_mm_free_contig(fifo->data);
			fifo->data = NULL;
		}

		oal_mm_free_contig(fifo);
	}
}

__attribute__((hot)) void *
fifo_peek(fifo_t *const fifo, uint32_t num)
{
	volatile void *ret = NULL;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!fifo)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (likely(num < fifo->depth)) {
		ret = fifo->data[num];
	}

	return (void *)ret;
}
