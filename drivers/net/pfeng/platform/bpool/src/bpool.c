// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2017-2020 NXP
 */

#include "oal.h"
#include "hal.h"
#include "fifo.h"
#include "bpool.h"

/*
 * 	Performance assumptions:
 * 		- Buffer space start address is aligned to at least cache line size.
 * 		- Every buffer is aligned at least to 256 bytes.
 * 		- Size of every buffer is power of two in range (256 <= size <= 4096).
 * 		- Descriptor space starts immediately after latest buffer so first descriptor is aligned to at least 256 bytes.
 * 		- Size of every descriptor is padded to integer multiple of cache line size.
 */

#define is_power_of_2(n) ((n) && !((n) & ((n) - 1)))

/**
 * @brief		Destroy pool and release all allocated memory
 * @param[in]	pool The bpool instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid argument
 */
__attribute__((cold)) errno_t
bpool_destroy(bpool_t *pool)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!pool)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*	Release FIFO */
	fifo_destroy((fifo_t *)(pool->free_fifo));
	pool->free_fifo = NULL;

	/*	Release mutex */
	oal_mutex_destroy(&pool->fifo_lock);

	/*	Release buffer memory block */
	oal_mm_free_contig(pool->block_origin_va);
	pool->block_origin_pa = NULL;
	pool->block_origin_va = NULL;
	pool->block_size = 0;

	/*	Release the pool itself */
	oal_mm_free_contig(pool);
	pool = NULL;

	return EOK;
}

/**
 * @brief		Get fill level
 * @param[in]	pool The bpool instance
 * @return		Number of entries within the pool
 */
__attribute__((hot)) errno_t
bpool_get_fill_level(bpool_t *pool, u32 *fill_level)
{
	errno_t ret;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!pool) || (!fill_level))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (unlikely(oal_mutex_lock(&pool->fifo_lock) != EOK))
		NXP_LOG_DEBUG("Mutex lock failed\n");

	ret = fifo_get_fill_level((fifo_t *)pool->free_fifo, fill_level);

	if (unlikely(oal_mutex_unlock(&pool->fifo_lock) != EOK))
		NXP_LOG_DEBUG("Mutex unlock failed\n");

	return ret;
}

/**
 * @brief		Get pool depth
 * @param[in]	pool The bpool instance
 * @return		Pool depth in number of entries
 */
__attribute__((pure, hot)) u32
bpool_get_depth(bpool_t *pool)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!pool)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return 0U;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	return pool->buffer_num;
}

/**
 * @brief		Get buffer from pool
 * @param[in]	pool The bpool instance
 * @return		Buffer VA or NULL if failed
 * @note		Is reentrant
 */
__attribute__((hot)) void *
bpool_get(bpool_t *pool)
{
	bpool_rx_buf_t *item;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!pool)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (unlikely(oal_mutex_lock(&pool->fifo_lock) != EOK))
		NXP_LOG_DEBUG("Mutex lock failed\n");

	item = fifo_get((fifo_t *)(pool->free_fifo));

	if (unlikely(oal_mutex_unlock(&pool->fifo_lock) != EOK))
		NXP_LOG_DEBUG("Mutex unlock failed\n");

	if (likely(item))
		return item->vaddr;

	return NULL;
}

/**
 * @brief		Put buffer back to the pool
 * @param[in]	pool Pool to put the buffer to
 * @param[in]	va Virtual address of the buffer to put
 * @note		Is reentrant
 */
__attribute__((hot)) void
bpool_put(bpool_t *pool, void *va)
{
#if defined(PFE_CFG_GET_ALL_ERRORS)
	errno_t ret;
#endif /* PFE_CFG_GET_ALL_ERRORS */
	bpool_rx_buf_t *item;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!pool)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	item = bpool_get_bd(pool, va);

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!item)) {
		NXP_LOG_ERROR("bpool_put: Failed to get bp\n");
		return;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

#ifdef BPOOL_CFG_MEM_BUF_WATCH
	if (NXP_MAGICINT != item->magicword)
		NXP_LOG_ERROR("%s: Memory region check failure\n", __func__);
#endif /* BPOOL_CFG_MEM_BUF_WATCH */

	if (unlikely(oal_mutex_lock(&pool->fifo_lock) != EOK)) {
		NXP_LOG_DEBUG("Mutex lock failed\n");
	}

#if defined(PFE_CFG_GET_ALL_ERRORS)
	if (unlikely(fifo_put((fifo_t *)(pool->free_fifo), item) != EOK))
		/*	Somehow we got more released buffers than is the FIFO capacity... */
		NXP_LOG_ERROR("Buffer pool overflow or FIFO does not exist\n");
#else
	(void)fifo_put((fifo_t *)(pool->free_fifo), item);
#endif /* PFE_CFG_GET_ALL_ERRORS */

	if (unlikely(oal_mutex_unlock(&pool->fifo_lock) != EOK))
		NXP_LOG_DEBUG("Mutex unlock failed\n");
}

/**
 * @brief		Allocate pool of RX buffers
 * @param[in]	depth Number of buffers in the pool
 * @param[in]	buf_size Size each buffer. Recommended to use power-of-two
 *              values, otherwise memory will be wasted for alignment.
 *              Maximal value is 4096.
 * @param[in]	align Physical alignment of particular buffers
 * @return		New pool instance or NULL if failed
 */
__attribute__((cold)) bpool_t *
bpool_create(u32 depth, u32 buf_size, u32 align)
{
	u32 i;
	bpool_rx_buf_t *fifo_item;
	void *paddr;
	void *vaddr;
	addr_t buf_paddr;
	addr_t buf_vaddr;
	bpool_t *the_pool;
	addr_t block_size;
	u32 aligned_buf_size;
	u32 real_buf_size;
	addr_t bd_addr;

	if (sizeof(bpool_rx_buf_t) % HAL_CACHE_LINE_SIZE)
		NXP_LOG_DEBUG("Sub-optimal structure size: buffer\n");

	if (align < HAL_CACHE_LINE_SIZE) {
		NXP_LOG_ERROR("Minimum buffer pool alignment is %d bytes\n",
			      HAL_CACHE_LINE_SIZE);
		return NULL;
	}

	if (is_power_of_2(align) == FALSE) {
		NXP_LOG_ERROR("Buffer pool alignment must be power of 2\n");
		return NULL;
	}

	if ((buf_size < 256U) || (buf_size > 4096U)) {
		NXP_LOG_ERROR(
			"Buffer size must be more than 256 and less than 4096 bytes\n");
		return NULL;
	}

	if (is_power_of_2(buf_size) == FALSE) {
		NXP_LOG_ERROR("Buffer size must be power of 2\n");
		return NULL;
	}

	/*	Due to buffer alignment required by ls1043a errata A-010022
		all buffers must be aligned like this:
		* 256 byte buffers aligned to 256
		* 512 byte buffers aligned to 512
		* 1024 byte buffers aligned to 1024
		* 2048 byte buffers aligned to 2048
		* 4096 byte buffers aligned to 4096
		* Bigger buffers must not be used!!
	*/
	if (buf_size > 2048U)
		/*	Maximal allowed size */
		aligned_buf_size = 4096U;
	else if (buf_size > 1024U)
		aligned_buf_size = 2048U;
	else if (buf_size > 512U)
		aligned_buf_size = 1024U;
	else if (buf_size > 256U)
		aligned_buf_size = 512U;
	else
		aligned_buf_size = 256U;

	/*	Beginning of each buffer is aligned to either 4096, 2048, 1024, 512, or 256
		=> it is practical to use those values also as buffer sizes. */

	if (0U != (aligned_buf_size % align)) {
		NXP_LOG_ERROR(
			"Failed to satisfy requested minimal alignment %u\n",
			align);
		return NULL;
	}
	real_buf_size = buf_size;

	/*	FIFO item structures of all buffers in the pool will be put into an array which
		will directly follow the buffers */
	block_size =
		(aligned_buf_size * depth) + (sizeof(bpool_rx_buf_t) * depth);

	/*	Allocate the buffer pool structure */
	the_pool = oal_mm_malloc_contig_aligned_cache(sizeof(bpool_t),
						      HAL_CACHE_LINE_SIZE);
	if (!the_pool) {
		NXP_LOG_ERROR("Memory allocation failed\n");
		return NULL;
	}

	if ((addr_t)the_pool % HAL_CACHE_LINE_SIZE) {
		NXP_LOG_DEBUG(
			"Sub-optimal structure alignment: bpool instance\n");
	}

	/*	Create FIFO as a container of the pool. Ensure the FIFO is 'protected' against
		concurrent accesses. */
	the_pool->free_fifo = fifo_create(depth);
	if (!the_pool->free_fifo) {
		NXP_LOG_ERROR("Can't create buffer FIFO\n");
		goto release_pool_and_fail;
	}

	if (oal_mutex_init(&the_pool->fifo_lock) != EOK) {
		NXP_LOG_ERROR("Mutex initialization failed\n");
		goto release_fifo_and_fail;
	}

	/*	Get physically contiguous memory region (buffers) */
	vaddr = oal_mm_malloc_contig_aligned_cache(block_size,
						   aligned_buf_size);
	if (!vaddr) {
		NXP_LOG_ERROR("Unable to get aligned memory block\n");
		goto release_mutex_and_fail;
	}

	/*	Get physical address */
	paddr = oal_mm_virt_to_phys_contig(vaddr);
	if (!paddr) {
		NXP_LOG_ERROR("Unable to get physical address\n");
		goto release_block_and_fail;
	}

	/*	Check alignment of physical address */
	if ((addr_t)paddr !=
	    ((addr_t)paddr & ~((addr_t)aligned_buf_size - 1U))) {
		NXP_LOG_ERROR(
			"The physical address p0x%p is not properly aligned to buffer size %u\n",
			paddr, aligned_buf_size);
		goto release_block_and_fail;
	}

	the_pool->block_origin_pa = paddr;
	the_pool->block_origin_va = vaddr;
	the_pool->buffer_align = aligned_buf_size;
	the_pool->buffer_num = depth;
	the_pool->buffer_raw_size = real_buf_size;
	the_pool->block_size = block_size;
	the_pool->block_pa_offset = (addr_t)vaddr - (addr_t)paddr;

	/*	Pre-compute addresses and offsets */

	/*	Buffer space */
	the_pool->buffer_pa_start = (addr_t)the_pool->block_origin_pa;
	the_pool->buffer_va_start = (addr_t)the_pool->block_origin_va;
	the_pool->buffer_pa_end =
		(the_pool->buffer_pa_start + (aligned_buf_size * depth) - 1);
	the_pool->buffer_va_end =
		(the_pool->buffer_va_start + (aligned_buf_size * depth) - 1);

	/*	Descriptor space */
	the_pool->bd_pa_start = the_pool->buffer_pa_end + 1;
	the_pool->bd_va_start = the_pool->buffer_va_end + 1;
	the_pool->bd_pa_end =
		(the_pool->bd_pa_start + (sizeof(bpool_rx_buf_t) * depth) - 1);
	the_pool->bd_va_end =
		(the_pool->bd_va_start + (sizeof(bpool_rx_buf_t) * depth) - 1);

	buf_paddr = (addr_t)paddr;
	buf_vaddr = (addr_t)vaddr;

	/*	Descriptors follows the buffers */
	bd_addr = the_pool->bd_va_start;

	/*	Fill the pool */
	for (i = 0U; i < depth; i++) {
		/*	Store buffer properties */
		fifo_item = (bpool_rx_buf_t *)(bd_addr);
		fifo_item->len = aligned_buf_size;
		fifo_item->paddr = (void *)buf_paddr;
		fifo_item->vaddr = (void *)buf_vaddr;

#ifdef BPOOL_CFG_MEM_BUF_WATCH
		fifo_item->magicword = NXP_MAGICINT;
#endif /* BPOOL_CFG_MEM_BUF_WATCH */

		if (fifo_put((fifo_t *)(the_pool->free_fifo), fifo_item)) {
			NXP_LOG_ERROR("Could not add buffer into the pool\n");
			goto release_block_and_fail;
		}

		/*	Move to next buffer */
		buf_paddr += (addr_t)aligned_buf_size;
		buf_vaddr += (addr_t)aligned_buf_size;
		bd_addr += sizeof(bpool_rx_buf_t);
	}

	NXP_LOG_DEBUG(
		"Buffer pool (%d buffers, %d bytes each) created @ p0x%p/v0x%p\n",
		the_pool->buffer_num, aligned_buf_size,
		(void *)the_pool->buffer_pa_start,
		(void *)the_pool->buffer_va_start);

	return the_pool;

release_block_and_fail:
	oal_mm_free_contig(vaddr);
release_mutex_and_fail:
	oal_mutex_destroy(&the_pool->fifo_lock);
release_fifo_and_fail:
	fifo_destroy((fifo_t *)(the_pool->free_fifo));
	the_pool->free_fifo = NULL;
release_pool_and_fail:
	oal_mm_free(the_pool);
	the_pool = NULL;

	return NULL;
}
