// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup  dxgr_PFE_PHY_IF
 * @{
 *
 * @file		pfe_phy_if.c
 * @brief		The PFE physical interface module source file.
 * @details		This file contains physical interface-related functionality.
 *
 */

#include "oal.h"
#include "hal.h"

#include "pfe_platform_cfg.h"
#include "pfe_cbus.h"
#include "pfe_ct.h"
#include "pfe_phy_if.h"
#include "linked_list.h"

typedef enum {
	PFE_PHY_IF_INVALID,
	PFE_PHY_IF_EMAC,
	PFE_PHY_IF_HIF
} pfe_phy_if_type_t;

struct __pfe_phy_if_tag {
	pfe_phy_if_type_t type;
	pfe_ct_phy_if_id_t id;
	char_t *name;
	pfe_class_t *class;
	addr_t dmem_base;
	pfe_ct_phy_if_t phy_if_class;
	LLIST_t log_ifs;
	oal_mutex_t lock;
	bool_t is_enabled;
	pfe_ct_block_state_t
		block_state; /* Copy of value in phy_if_class for faster access */
	union {
		pfe_emac_t *emac;
		pfe_hif_chnl_t *hif_ch;
		void *instance;
	} port;
};

typedef struct __pfe_phy_if_list_entry_tag {
	pfe_log_if_t *log_if;
	LLIST_t iterator;
} pfe_phy_if_list_entry_t;

static errno_t pfe_phy_if_write_to_class_nostats(pfe_phy_if_t *iface,
						 pfe_ct_phy_if_t *class_if);
static errno_t pfe_phy_if_write_to_class(pfe_phy_if_t *iface,
					 pfe_ct_phy_if_t *class_if);
static bool_t pfe_phy_if_has_log_if_nolock(pfe_phy_if_t *iface,
					   pfe_log_if_t *log_if);
static bool_t pfe_phy_if_has_enabled_log_if_nolock(pfe_phy_if_t *iface);
static bool_t pfe_phy_if_has_promisc_log_if_nolock(pfe_phy_if_t *iface);
static errno_t pfe_phy_if_disable_nolock(pfe_phy_if_t *iface);
static uint32_t pfe_phy_if_stat_to_str(pfe_ct_phy_if_stats_t *stat, char *buf,
				       u32 buf_len, uint8_t verb_level);

/**
 * @brief		Write interface structure to classifier memory skipping interface statistics
 * @param[in]	iface The interface instance
 * @param[in]	class_if Pointer to the structure to be written
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 */
static errno_t
pfe_phy_if_write_to_class_nostats(pfe_phy_if_t *iface,
				  pfe_ct_phy_if_t *class_if)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!class_if) || (!iface) ||
		     (iface->dmem_base == 0U))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/* Be sure that phy_stats are at correct place */
	_ct_assert((sizeof(pfe_ct_phy_if_t) - sizeof(pfe_ct_phy_if_stats_t)) ==
		   offsetof(pfe_ct_phy_if_t, phy_stats));

	/*	Write to DMEM of ALL PEs */
	return pfe_class_write_dmem(
		iface->class, -1, (void *)iface->dmem_base, class_if,
		sizeof(pfe_ct_phy_if_t) - sizeof(pfe_ct_phy_if_stats_t));
}

/**
 * @brief		Write interface structure to classifier memory with statistics
 * @param[in]	iface The interface instance
 * @param[in]	class_if Pointer to the structure to be written
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 */
static errno_t
pfe_phy_if_write_to_class(pfe_phy_if_t *iface, pfe_ct_phy_if_t *class_if)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!class_if) || (!iface) ||
		     (iface->dmem_base == 0U))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*	Write to DMEM of ALL PEs */
	return pfe_class_write_dmem(iface->class, -1, (void *)iface->dmem_base,
				    class_if, sizeof(pfe_ct_phy_if_t));
}

/**
 * @brief		Converts statistics of a physical interface or classification algorithm into a text form
 * @param[in]	stat		Statistics to convert
 * @param[out]	buf			Buffer where to write the text
 * @param[in]	buf_len		Buffer length
 * @param[in]	verb_level	Verbosity level
 * @return		Number of bytes written into the output buffer
 */
static uint32_t
pfe_phy_if_stat_to_str(pfe_ct_phy_if_stats_t *stat, char *buf, uint32_t buf_len,
		       uint8_t verb_level)
{
	uint32_t len = 0U;

	(void)verb_level;
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!stat) || (!buf))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return 0U;
	}
#endif
	len += oal_util_snprintf(buf + len, buf_len - len,
				 "Ingress frames:   %u\n",
				 oal_ntohl(stat->ingress));
	len += oal_util_snprintf(buf + len, buf_len - len,
				 "Egress frames:    %u\n",
				 oal_ntohl(stat->egress));
	len += oal_util_snprintf(buf + len, buf_len - len,
				 "Malformed frames: %u\n",
				 oal_ntohl(stat->malformed));
	len += oal_util_snprintf(buf + len, buf_len - len,
				 "Discarded frames: %u\n",
				 oal_ntohl(stat->discarded));
	return len;
}

/**
 * @brief		Create new physical interface instance
 * @param[in]	class The classifier instance
 * @param[in]	id The PFE firmware is using HW interface identifiers to distinguish
 * 				between particular interfaces. The set of available IDs (the
 * 				pfe_ct_phy_if_id_t) shall remain compatible with the firmware.
 * @param[in]	name Name of the interface
 * @return		The interface instance or NULL if failed
 */
pfe_phy_if_t *
pfe_phy_if_create(pfe_class_t *class, pfe_ct_phy_if_id_t id, char_t *name)
{
	pfe_phy_if_t *iface;
	pfe_ct_pe_mmap_t pfe_pe_mmap = { 0U };

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!class)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	iface = oal_mm_malloc(sizeof(pfe_phy_if_t));

	if (!iface) {
		return NULL;
	} else {
		memset(iface, 0, sizeof(pfe_phy_if_t));
		iface->type = PFE_PHY_IF_INVALID;
		iface->id = id;
		iface->class = class;
		iface->is_enabled = FALSE;
		LLIST_Init(&iface->log_ifs);

		/*	Get mmap base from PE[0] since all PEs have the same memory map */
		if (pfe_class_get_mmap(class, 0U, &pfe_pe_mmap) != EOK) {
			NXP_LOG_ERROR("Could not get memory map\n");
			goto free_and_fail;
		}

		if (oal_ntohl(pfe_pe_mmap.dmem_phy_if_size) <
		    ((1U + id) * sizeof(pfe_ct_phy_if_t))) {
			NXP_LOG_ERROR("PhyIf storage is too small\n");
			goto free_and_fail;
		}

		/*	Get physical interface instance address within DMEM array */
		iface->dmem_base = oal_ntohl(pfe_pe_mmap.dmem_phy_if_base) +
				   (id * sizeof(pfe_ct_phy_if_t));

		if (oal_mutex_init(&iface->lock) != EOK) {
			NXP_LOG_ERROR("Could not initialize mutex\n");
			oal_mm_free(iface);
			iface = NULL;
			return NULL;
		}

		if (!name) {
			iface->name = NULL;
		} else {
			iface->name = oal_mm_malloc(strlen(name) + 1U);
			strcpy(iface->name, name);
		}

		/*	Initialize the interface structure in classifier */
		iface->phy_if_class.id = id;
		iface->phy_if_class.block_state = IF_BS_FORWARDING;
		iface->phy_if_class.mirror = PFE_PHY_IF_ID_INVALID;

		/* Be sure that statistics are zeroed (endianness doesn't mater for this) */
		iface->phy_if_class.phy_stats.ingress = 0;
		iface->phy_if_class.phy_stats.egress = 0;
		iface->phy_if_class.phy_stats.discarded = 0;
		iface->phy_if_class.phy_stats.malformed = 0;

		/*	Write the configuration to classifier */
		if (EOK !=
		    pfe_phy_if_write_to_class(iface, &iface->phy_if_class)) {
			NXP_LOG_ERROR("Phy IF configuration failed\n");
			oal_mm_free(iface);
			iface = NULL;
		}
	}

	return iface;

free_and_fail:
	pfe_phy_if_destroy(iface);
	return NULL;
}

/**
 * @brief		Destroy interface instance
 * @param[in]	iface The interface instance
 * @return		EOK success, error code otherwise
 */
errno_t
pfe_phy_if_destroy(pfe_phy_if_t *iface)
{
	errno_t ret = EOK;

	if (iface) {
		if (oal_mutex_lock(&iface->lock) != EOK) {
			NXP_LOG_DEBUG("mutex lock failed\n");
		}

		if (LLIST_IsEmpty(&iface->log_ifs) == FALSE) {
			/*	Do not allow orphaned logical interfaces */
			NXP_LOG_WARNING(
				"%s still contains logical interfaces. Destroy them first.\n",
				iface->name);
			ret = EPERM;
		}

		if (oal_mutex_unlock(&iface->lock) != EOK) {
			NXP_LOG_DEBUG("mutex unlock failed\n");
		}

		if (ret == EOK) {
			if (iface->name) {
				oal_mm_free(iface->name);
				iface->name = NULL;
			}

			if (oal_mutex_destroy(&iface->lock) != EOK) {
				NXP_LOG_DEBUG("Could not destroy mutex\n");
			}

			oal_mm_free(iface);
		}
	}

	return ret;
}

/**
 * @brief		Return classifier instance associated with interface
 * @param[in]	iface The interface instance
 * @return		The classifier instance
 */
__attribute__((pure)) pfe_class_t *
pfe_phy_if_get_class(pfe_phy_if_t *iface)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	return iface->class;
}

/**
 * @brief		Add logical interface
 * @details		First added logical interface will become the default one. Default is used
 * 				when packet is not matching any other logical interface within the physical one.
 * @param[in]	iface The physical interface instance
 * @param[in]	log_if The logical interface instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 * @retval		ENOEXEC Command failed
 * @retval		EEXIST Entry exists
 * @note		API to be used only by pfe_log_if module
 */
errno_t
pfe_phy_if_add_log_if(pfe_phy_if_t *iface, pfe_log_if_t *log_if)
{
	pfe_phy_if_list_entry_t *entry, *tmp_entry;
	addr_t log_if_dmem_base = 0U;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!iface) || (!log_if))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	entry = oal_mm_malloc(sizeof(pfe_phy_if_list_entry_t));
	if (!entry) {
		NXP_LOG_DEBUG("Memory allocation failed\n");
		return ENOMEM;
	}

	entry->log_if = log_if;

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	if (LLIST_IsEmpty(&iface->log_ifs) == TRUE) {
		/*
			No logical interface assigned yet
		*/

		/*	Get DMEM address to the logical interface structure */
		if (EOK !=
		    pfe_log_if_get_dmem_base(log_if, &log_if_dmem_base)) {
			NXP_LOG_ERROR(
				"Could not get DMEM base (%s, parent: %s)\n",
				pfe_log_if_get_name(log_if), iface->name);

			goto unlock_and_fail;
		}

#if defined(PFE_CFG_NULL_ARG_CHECK)
		if (log_if_dmem_base == 0U) {
			NXP_LOG_ERROR("LogIf base is NULL (%s)\n",
				      pfe_log_if_get_name(log_if));
			goto unlock_and_fail;
		}
#endif /* PFE_CFG_NULL_ARG_CHECK */

		/*	First added interface will become the default one */
		iface->phy_if_class.def_log_if =
			oal_htonl((uint32_t)log_if_dmem_base);
	} else {
		/*
			Chain new logIf in (at the begin) => modify first entry .next pointer
		*/

		/*	Check duplicates */
		if (pfe_phy_if_has_log_if_nolock(iface, log_if) == TRUE) {
			NXP_LOG_WARNING("%s already added\n",
					pfe_log_if_get_name(log_if));
			if (oal_mutex_unlock(&iface->lock) != EOK) {
				NXP_LOG_DEBUG("mutex unlock failed\n");
			}

			return EEXIST;
		}

		/*	Get current first item of the list */
		tmp_entry = LLIST_Data(iface->log_ifs.prNext,
				       pfe_phy_if_list_entry_t, iterator);

		/*	Get address of the first item in DMEM */
		log_if_dmem_base = 0U;
		if (EOK != pfe_log_if_get_dmem_base(tmp_entry->log_if,
						    &log_if_dmem_base)) {
			NXP_LOG_ERROR(
				"Could not get DMEM base (%s, parent: %s)\n",
				pfe_log_if_get_name(tmp_entry->log_if),
				iface->name);

			goto unlock_and_fail;
		}

#if defined(PFE_CFG_NULL_ARG_CHECK)
		if (log_if_dmem_base == 0U) {
			NXP_LOG_ERROR("LogIf base is NULL (%s)\n",
				      pfe_log_if_get_name(tmp_entry->log_if));
			goto unlock_and_fail;
		}
#endif /* PFE_CFG_NULL_ARG_CHECK */

		/*	Change 'next' pointer of the new entry */
		if (EOK !=
		    pfe_log_if_set_next_dmem_ptr(log_if, log_if_dmem_base)) {
			NXP_LOG_ERROR(
				"Can't set next linked list pointer (%s, parent: %s)\n",
				pfe_log_if_get_name(log_if), iface->name);

			goto unlock_and_fail;
		}
	}

	/*	Get DMEM pointer to the new logIf */
	log_if_dmem_base = 0U;
	if (pfe_log_if_get_dmem_base(log_if, &log_if_dmem_base) != EOK) {
		NXP_LOG_ERROR(
			"Could not get logIf DMEM base (%s, parent: %s)\n",
			pfe_log_if_get_name(log_if), iface->name);

		goto unlock_and_fail;
	}

	/*	Set list head to the new logIf */
	iface->phy_if_class.log_ifs =
		oal_htonl(PFE_CFG_CLASS_ELF_DMEM_BASE |
			  (log_if_dmem_base & (PFE_CFG_CLASS_DMEM_SIZE - 1U)));

	/*	Store physical interface changes (.phy_if_class) to DMEM */
	if (EOK !=
	    pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class)) {
		NXP_LOG_ERROR("Unable to update structure in DMEM (%s)\n",
			      iface->name);
		goto unlock_and_fail;
	} else {
		/*	Now the new logIf is head of the list and classifier will see that */
		NXP_LOG_DEBUG("%s (p0x%p) added to %s (p0x%p)\n",
			      pfe_log_if_get_name(log_if),
			      (void *)log_if_dmem_base, iface->name,
			      (void *)iface->dmem_base);
	}

	/*	Add instance to local list of logical interfaces */
	LLIST_AddAtBegin(&entry->iterator, &iface->log_ifs);

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return EOK;

unlock_and_fail:
	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ENOEXEC;
}

static bool_t
pfe_phy_if_has_log_if_nolock(pfe_phy_if_t *iface, pfe_log_if_t *log_if)
{
	LLIST_t *item;
	pfe_phy_if_list_entry_t *entry;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!iface) || (!log_if))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return FALSE;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	LLIST_ForEach(item, &iface->log_ifs)
	{
		entry = LLIST_Data(item, pfe_phy_if_list_entry_t, iterator);
		if (log_if == entry->log_if) {
			return TRUE;
		}
	}

	return FALSE;
}

static bool_t
pfe_phy_if_has_enabled_log_if_nolock(pfe_phy_if_t *iface)
{
	LLIST_t *item;
	pfe_phy_if_list_entry_t *entry;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return FALSE;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	LLIST_ForEach(item, &iface->log_ifs)
	{
		entry = LLIST_Data(item, pfe_phy_if_list_entry_t, iterator);
		if (pfe_log_if_is_enabled(entry->log_if) == TRUE) {
			return TRUE;
		}
	}

	return FALSE;
}

static bool_t
pfe_phy_if_has_promisc_log_if_nolock(pfe_phy_if_t *iface)
{
	LLIST_t *item;
	pfe_phy_if_list_entry_t *entry;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return FALSE;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	LLIST_ForEach(item, &iface->log_ifs)
	{
		entry = LLIST_Data(item, pfe_phy_if_list_entry_t, iterator);
		if ((pfe_log_if_is_enabled(entry->log_if) == TRUE) &&
		    (pfe_log_if_is_promisc(entry->log_if) == TRUE)) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * @brief		Check if physical interface contains given logical interface
 * @param[in]	iface The physical interface instance
 * @param[in]	log_if The logical interface instance
 * @return		TRUE if logical interface is bound to the physical one. False
 * 				otherwise.
 */
bool_t
pfe_phy_if_has_log_if(pfe_phy_if_t *iface, pfe_log_if_t *log_if)
{
	bool_t match = FALSE;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!iface) || (!log_if))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return FALSE;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	match = pfe_phy_if_has_log_if_nolock(iface, log_if);

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return match;
}

/**
 * @brief		Delete associated logical interface
 * @param[in]	iface The physical interface instance
 * @param[in]	log_if The logical interface instance to be deleted
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 * @retval		ENOEXEC Command failed
 * @retval		ENOENT Entry not found
 * @note		API to be used only by pfe_log_if module
 */
errno_t
pfe_phy_if_del_log_if(pfe_phy_if_t *iface, pfe_log_if_t *log_if)
{
	pfe_phy_if_list_entry_t *entry = NULL, *prev_entry = NULL;
	LLIST_t *item;
	bool_t found = FALSE;
	addr_t log_if_dmem_base = 0U, next_dmem_ptr = 0U;
	errno_t ret;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!iface) || (!log_if))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	LLIST_ForEach(item, &iface->log_ifs)
	{
		entry = LLIST_Data(item, pfe_phy_if_list_entry_t, iterator);
		if (log_if == entry->log_if) {
			found = TRUE;
			break;
		} else {
			prev_entry = entry;
		}
	}

	if (found == FALSE) {
		NXP_LOG_WARNING("%s not found in %s\n",
				pfe_log_if_get_name(log_if), iface->name);
		if (oal_mutex_unlock(&iface->lock) != EOK) {
			NXP_LOG_DEBUG("mutex unlock failed\n");
		}

		return ENOENT;
	}

	/*
		Bypass the entry within the linked list in DMEM
	*/
	next_dmem_ptr = 0U;
	if (EOK !=
	    pfe_log_if_get_next_dmem_ptr(entry->log_if, &next_dmem_ptr)) {
		NXP_LOG_ERROR("Could not get DMEM base (%s, parent: %s)\n",
			      pfe_log_if_get_name(entry->log_if), iface->name);

		goto unlock_and_fail;
	}

	if (!prev_entry) {
		/*
			First in list
		*/

		if (next_dmem_ptr == 0U) {
			/*	No next entry, no previous entry. Just remove. */
			NXP_LOG_WARNING(
				"Removing default logical interface (%s, parent: %s)\n",
				pfe_log_if_get_name(entry->log_if),
				iface->name);

			/*	Invalidate head and default interface */
			iface->phy_if_class.def_log_if =
				oal_htonl((uint32_t)0U);
			iface->phy_if_class.log_ifs = oal_htonl((uint32_t)0U);
		} else {
			/*	Next pointer is OK, just move the head. Default interface is the latest one so no change here. */
			iface->phy_if_class.log_ifs =
				oal_htonl((uint32_t)next_dmem_ptr);
		}
	} else {
		/*
			Previous entry needs to be updated
		*/

		/*	Set 'next' pointer of previous entry to 'next' pointer of deleted entry */
		if (EOK != pfe_log_if_set_next_dmem_ptr(prev_entry->log_if,
							next_dmem_ptr)) {
			NXP_LOG_ERROR(
				"Can't set next linked list pointer (%s, parent: %s)\n",
				pfe_log_if_get_name(prev_entry->log_if),
				iface->name);

			goto unlock_and_fail;
		}

		/*	If 'next' pointer of deleted entry is NULL then we're removing default interface */
		if (next_dmem_ptr == 0U) {
			NXP_LOG_INFO(
				"Removing default logical interface (%s, parent: %s). Will be replaced by %s.\n",
				pfe_log_if_get_name(log_if), iface->name,
				pfe_log_if_get_name(prev_entry->log_if));

			log_if_dmem_base = 0U;
			if (EOK !=
			    pfe_log_if_get_dmem_base(prev_entry->log_if,
						     &log_if_dmem_base)) {
				NXP_LOG_ERROR(
					"Could not get DMEM base (%s, parent: %s)\n",
					pfe_log_if_get_name(prev_entry->log_if),
					iface->name);

				/*	Don't leave here as the previous entry is set up to bypass the deleted entry */
			}

			iface->phy_if_class.def_log_if =
				oal_htonl((uint32_t)log_if_dmem_base);
		}
	}

	/*	Store physical interface changes (.phy_if_class) to DMEM */
	if (EOK !=
	    pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class)) {
		/*	TODO: How to handle this? */
		NXP_LOG_ERROR("Unable to update structure in DMEM (%s)\n",
			      iface->name);
		goto unlock_and_fail;
	} else {
		/*	Now the change is visible to classifier */
		log_if_dmem_base = 0U;
		if (EOK !=
		    pfe_log_if_get_dmem_base(log_if, &log_if_dmem_base)) {
			NXP_LOG_ERROR(
				"Could not get DMEM base (%s, parent: %s)\n",
				pfe_log_if_get_name(log_if), iface->name);

			/*	Don't leave here as the previous entry is set up to bypass the deleted entry */
		}

		NXP_LOG_INFO("%s (p0x%p) removed from %s (p0x%p)\n",
			     pfe_log_if_get_name(log_if),
			     (void *)log_if_dmem_base, iface->name,
			     (void *)iface->dmem_base);
	}

	/*	Remove entry from local list */
	LLIST_Remove(&entry->iterator);

	/*	Release the entry */
	oal_mm_free(entry);
	entry = NULL;

	/*	Disable the interface in case that there are not enabled logical interfaces */
	ret = pfe_phy_if_disable_nolock(iface);
	if (ret != EOK) {
		NXP_LOG_ERROR("%s can't be disabled: %d\n", iface->name, ret);
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;

unlock_and_fail:
	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ENOEXEC;
}

/**
 * @brief Set the block state
 * @param[in] iface The interface instance
 * @param[out] block_state Block state to set
 * @return EOK on success or an error code
 */
errno_t
pfe_phy_if_set_block_state(pfe_phy_if_t *iface,
			   pfe_ct_block_state_t block_state)
{
	errno_t ret = EOK;
	pfe_ct_block_state_t tmp;
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}
	/* Set the requested state */
	tmp = iface->block_state;
	iface->block_state = block_state;
	iface->phy_if_class.block_state = block_state;
	/* Write changes into the HW */
	ret = pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class);

	if (ret != EOK) { /* Failure to update the HW */
		/* Restore previous value */
		iface->block_state = tmp;
		iface->phy_if_class.block_state = tmp;
		/* Report an error */
		NXP_LOG_DEBUG("Can't write PHY IF structure to classifier\n");
		ret = EINVAL;
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_ERROR("mutex unlock failed\n");
	}
	return ret;
}

/**
 * @brief Get the block state
 * @param[in] iface The interface instance
 * @param[out] block_state Current block state
 * @return EOK On success or an error code
 */
errno_t
pfe_phy_if_get_block_state(pfe_phy_if_t *iface,
			   pfe_ct_block_state_t *block_state)
{
	errno_t ret = EOK;
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	/* The value is being stored in the iface structure and kept up-to-date
	   with the value in PEs (HW) thus it can be simply returned */
	*block_state = iface->block_state;

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_ERROR("mutex unlock failed\n");
	}
	return ret;
}
/**
 * @brief		Get operational mode
 * @param[in]	iface The interface instance
 * @retval		Current phy_if mode. See pfe_ct_if_op_mode_t.
 */
pfe_ct_if_op_mode_t
pfe_phy_if_get_op_mode(pfe_phy_if_t *iface)
{
	pfe_ct_if_op_mode_t ret;

	/*	Update the interface structure */
	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	ret = iface->phy_if_class.mode;

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Set operational mode
 * @param[in]	iface The interface instance
 * @param[in]	mode Mode to be set. See pfe_ct_if_op_mode_t.
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 */
errno_t
pfe_phy_if_set_op_mode(pfe_phy_if_t *iface, pfe_ct_if_op_mode_t mode)
{
	pfe_ct_pe_mmap_t mmap;
	errno_t ret;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*	Get memory map */
	ret = pfe_class_get_mmap(iface->class, 0U, &mmap);
	if (ret != EOK) {
		NXP_LOG_DEBUG("Can't get memory map\n");
		return EINVAL;
	}

	/*	Update the interface structure */
	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	iface->phy_if_class.mode = mode;
	ret = pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class);
	if (ret != EOK) {
		NXP_LOG_DEBUG("Can't write PHY IF structure to classifier\n");
		ret = EINVAL;
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief Set mirroring on the given interface
 * @param[in] iface The interface which traffic shall be mirrored
 * @param[in] mirror The ID of the interface which shall send out the mirrored traffic.
 *                   The value PFE_PHY_IF_ID_INVALID disables the feature.
 * @return EOK on success or an error code.
 */
errno_t
pfe_phy_if_set_mirroring(pfe_phy_if_t *iface, pfe_ct_phy_if_id_t mirror)
{
	pfe_ct_pe_mmap_t mmap;
	errno_t ret;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*	Get memory map */
	ret = pfe_class_get_mmap(iface->class, 0U, &mmap);
	if (ret != EOK) {
		NXP_LOG_DEBUG("Can't get memory map\n");
		return EINVAL;
	}

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	iface->phy_if_class.mirror = mirror;

	ret = pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class);
	if (ret != EOK) {
		NXP_LOG_DEBUG("Can't write PHY IF structure to classifier\n");
		ret = EINVAL;
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}
	return ret;
}

/**
 * @brief Get mirroring configuration on the given interface
 * @param[in] iface The interface which traffic shall be mirrored
 * @return The ID of the interface where is the traffic mirrored
 *         (PFE_PHY_IF_ID_INVALID if is disabled).
 */
pfe_ct_phy_if_id_t
pfe_phy_if_get_mirroring(pfe_phy_if_t *iface)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return PFE_PHY_IF_ID_INVALID;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	return iface->phy_if_class.mirror;
}

/**
 * @brief		Bind interface with EMAC
 * @param[in]	iface The interface instance
 * @param[in]	emac The EMAC instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 * @retval		EPERM Operation not permitted
 */
errno_t
pfe_phy_if_bind_emac(pfe_phy_if_t *iface, pfe_emac_t *emac)
{
	errno_t ret = EOK;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!emac) || (!iface))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	if (iface->type == PFE_PHY_IF_INVALID) {
		iface->type = PFE_PHY_IF_EMAC;
		iface->port.emac = emac;

		if (iface->is_enabled == TRUE) {
			if (oal_mutex_unlock(&iface->lock) != EOK) {
				NXP_LOG_DEBUG("mutex unlock failed\n");
			}

			ret = pfe_phy_if_enable(iface);
		} else {
			if (oal_mutex_unlock(&iface->lock) != EOK) {
				NXP_LOG_DEBUG("mutex unlock failed\n");
			}

			ret = pfe_phy_if_disable(iface);
		}
	} else {
		NXP_LOG_DEBUG("Interface already bound\n");

		if (oal_mutex_unlock(&iface->lock) != EOK) {
			NXP_LOG_DEBUG("mutex unlock failed\n");
		}

		ret = EPERM;
	}

	return ret;
}

/**
 * @brief		Get associated EMAC instance
 * @param[in]	iface The interface instance
 * @return		Associated EMAC instance or NULL if failed
 */
pfe_emac_t *
pfe_phy_if_get_emac(pfe_phy_if_t *iface)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (iface->type == PFE_PHY_IF_EMAC) {
		return iface->port.emac;
	} else {
		NXP_LOG_DEBUG("Invalid interface type\n");
		return NULL;
	}
}

/**
 * @brief		Bind interface with HIF channel
 * @param[in]	iface The interface instance
 * @param[in]	hif The HIF channel instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 * @retval		EPERM Operation not permitted
 */
errno_t
pfe_phy_if_bind_hif(pfe_phy_if_t *iface, pfe_hif_chnl_t *hif)
{
	errno_t ret = EOK;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!hif) || (!iface))) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	if (iface->type == PFE_PHY_IF_INVALID) {
		iface->type = PFE_PHY_IF_HIF;
		iface->port.hif_ch = hif;
	} else {
		NXP_LOG_DEBUG("Interface already bound\n");
		ret = EPERM;
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	return ret;
}

/**
 * @brief		Get associated HIF channel instance
 * @param[in]	iface The interface instance
 * @return		Associated HIF channel instance or NULL if failed
 */
pfe_hif_chnl_t *
pfe_phy_if_get_hif(pfe_phy_if_t *iface)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (iface->type == PFE_PHY_IF_HIF) {
		return iface->port.hif_ch;
	} else {
		NXP_LOG_DEBUG("Invalid interface type\n");
		return NULL;
	}
}

/**
 * @brief		Check if interface is enabled
 * @param[in]	iface The interface instance
 * @retval		TRUE if enabled
 * @retval		FALSE if disabled
 */
bool_t
pfe_phy_if_is_enabled(pfe_phy_if_t *iface)
{
	bool_t ret = FALSE;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	ret = iface->is_enabled;

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}
	return ret;
}

/**
 * @brief		Enable interface (RX/TX)
 * @param[in]	iface The interface instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 */
errno_t
pfe_phy_if_enable(pfe_phy_if_t *iface)
{
	errno_t ret = EOK;
	pfe_ct_if_flags_t tmp;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	NXP_LOG_DEBUG("Enabling %s\n", iface->name);

	/*	Enable interface instance. Backup flags and write the changes. */
	tmp = iface->phy_if_class.flags;
	iface->phy_if_class.flags |= IF_FL_ENABLED;
	ret = pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class);
	if (ret != EOK) {
		/*	Failed. Revert flags. */
		NXP_LOG_ERROR("Phy IF configuration failed\n");
		iface->phy_if_class.flags = tmp;
	} else {
		/*	Mark the interface as enabled */
		iface->is_enabled = TRUE;

		/*	Enable also associated HW block */
		if (!iface->port.instance) {
			/*	No HW block associated */
			;
		} else {
			if (iface->type == PFE_PHY_IF_EMAC) {
				pfe_emac_enable(iface->port.emac);
			} else if (iface->type == PFE_PHY_IF_HIF) {
				ret = pfe_hif_chnl_rx_enable(
					iface->port.hif_ch);
				if (ret != EOK) {
					NXP_LOG_DEBUG(
						"Can't enable HIF channel RX: %d\n",
						ret);
				} else {
					ret = pfe_hif_chnl_tx_enable(
						iface->port.hif_ch);
					if (ret != EOK) {
						NXP_LOG_DEBUG(
							"Can't enable HIF channel TX: %d\n",
							ret);
					}
				}
			} else {
				NXP_LOG_DEBUG("Invalid interface type\n");
				ret = EINVAL;
			}
		}

		if (ret != EOK) {
			/*	HW configuration failure. Backup flags and disable the instance. */
			tmp = iface->phy_if_class.flags;
			iface->phy_if_class.flags &= ~IF_FL_ENABLED;
			ret = pfe_phy_if_write_to_class_nostats(
				iface, &iface->phy_if_class);
			if (ret != EOK) {
				/*	Failed. Revert flags. */
				NXP_LOG_ERROR("Phy IF configuration failed\n");
				iface->phy_if_class.flags = tmp;
			} else {
				iface->is_enabled = FALSE;
			}
		}
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

static errno_t
pfe_phy_if_disable_nolock(pfe_phy_if_t *iface)
{
	errno_t ret = EOK;
	pfe_ct_if_flags_t tmp;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*
		Go through all associated logical interfaces and search
		for enabled ones. If there is some enabled logical
		interface, don't disable the physical one.
	*/
	if (pfe_phy_if_has_enabled_log_if_nolock(iface) == TRUE) {
		return EOK;
	}

	NXP_LOG_DEBUG("Disabling %s\n", iface->name);

	/*	Disable interface instance. Backup flags and write the changes. */
	tmp = iface->phy_if_class.flags;
	iface->phy_if_class.flags &= ~IF_FL_ENABLED;
	ret = pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class);
	if (ret != EOK) {
		/*	Failed. Revert flags. */
		NXP_LOG_ERROR("Phy IF configuration failed\n");
		iface->phy_if_class.flags = tmp;
	} else {
		/*	Mark the interface as disabled */
		iface->is_enabled = FALSE;

		/*	Disable also associated HW block */
		if (!iface->port.instance) {
			/*	No HW block associated */
			;
		} else {
			if (iface->type == PFE_PHY_IF_EMAC) {
				pfe_emac_disable(iface->port.emac);
			} else if (iface->type == PFE_PHY_IF_HIF) {
				pfe_hif_chnl_rx_disable(iface->port.hif_ch);
				pfe_hif_chnl_tx_disable(iface->port.hif_ch);
			} else {
				NXP_LOG_DEBUG("Invalid interface type\n");
				ret = EINVAL;
			}
		}
	}

	return ret;
}

/**
 * @brief		Disable interface (RX/TX)
 * @param[in]	iface The interface instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 */
errno_t
pfe_phy_if_disable(pfe_phy_if_t *iface)
{
	errno_t ret;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	ret = pfe_phy_if_disable_nolock(iface);

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Check if phy_if in promiscuous mode
 * @param[in]	iface The interface instance
 * @retval		TRUE promiscuous mode is enabled
 * @retval		FALSE  promiscuous mode is disbaled
 */
bool_t
pfe_phy_if_is_promisc(pfe_phy_if_t *iface)
{
	bool_t ret = FALSE;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	ret = (0 != (iface->phy_if_class.flags & IF_FL_PROMISC));

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}
	return ret;
}

/**
 * @brief		Enable promiscuous mode
 * @param[in]	iface The interface instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 */
errno_t
pfe_phy_if_promisc_enable(pfe_phy_if_t *iface)
{
	errno_t ret = EOK;
	pfe_ct_if_flags_t tmp;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	/*	Enable instance promiscuous mode. Backup flags and write the changes. */
	tmp = iface->phy_if_class.flags;
	iface->phy_if_class.flags |= IF_FL_PROMISC;
	ret = pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class);
	if (ret != EOK) {
		/*	Failed. Revert flags. */
		NXP_LOG_ERROR("Phy IF configuration failed\n");
		iface->phy_if_class.flags = tmp;
	} else {
		/*	Set up also associated HW block */
		if (!iface->port.instance) {
			/*	No HW block associated */
			;
		} else {
			if (iface->type == PFE_PHY_IF_EMAC) {
				pfe_emac_enable_promisc_mode(iface->port.emac);
			} else if (iface->type == PFE_PHY_IF_HIF) {
				/*	HIF does not offer filtering ability */
				;
			} else {
				NXP_LOG_ERROR("Invalid interface type\n");
				ret = EINVAL;
			}
		}
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Disable promiscuous mode
 * @param[in]	iface The interface instance
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 */
errno_t
pfe_phy_if_promisc_disable(pfe_phy_if_t *iface)
{
	errno_t ret = EOK;
	pfe_ct_if_flags_t tmp;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	/*
		Go through all associated logical interfaces and search
		for promiscuous ones. If there is some enabled promiscuous
		logical interface, don't disable promiscuous mode on the
		physical one.
	*/
	if (pfe_phy_if_has_promisc_log_if_nolock(iface) == TRUE) {
		NXP_LOG_INFO(
			"%s primiscuous mode not disabled since contains promiscuous logical interface(s)\n",
			iface->name);

		if (oal_mutex_unlock(&iface->lock) != EOK) {
			NXP_LOG_DEBUG("mutex unlock failed\n");
		}

		return EOK;
	}

	/*	Disable instance promiscuous mode. Backup flags and write the changes. */
	tmp = iface->phy_if_class.flags;
	iface->phy_if_class.flags &= ~IF_FL_PROMISC;
	ret = pfe_phy_if_write_to_class_nostats(iface, &iface->phy_if_class);
	if (ret != EOK) {
		/*	Failed. Revert flags. */
		NXP_LOG_ERROR("Phy IF configuration failed\n");
		iface->phy_if_class.flags = tmp;
	} else {
		/*	Set up also associated HW block */
		if (!iface->port.instance) {
			/*	No HW block associated */
			;
		} else {
			if (iface->type == PFE_PHY_IF_EMAC) {
				pfe_emac_disable_promisc_mode(iface->port.emac);
			} else if (iface->type == PFE_PHY_IF_HIF) {
				/*	HIF does not offer filtering ability */
				;
			} else {
				NXP_LOG_ERROR("Invalid interface type\n");
				ret = EINVAL;
			}
		}
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Add MAC address
 * @param[in]	iface The interface instance
 * @param[in]	addr The MAC address to add
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 * @retval		ENOEXEC Command failed
 */
errno_t
pfe_phy_if_add_mac_addr(pfe_phy_if_t *iface, pfe_mac_addr_t addr)
{
	errno_t ret = EOK;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	/*	Configure also associated HW block */
	if (!iface->port.instance) {
		/*	No HW block associated */
		;
	} else {
		if (iface->type == PFE_PHY_IF_EMAC) {
			ret = pfe_emac_add_addr(iface->port.emac, addr);
			if (ret == ENOSPC) {
				NXP_LOG_INFO(
					"No space left in MAC ADDR exact match table, adding to hash group\n");
			} else if (ret != EOK) {
				NXP_LOG_ERROR("Unable to add MAC address: %d\n",
					      ret);
				ret = ENOEXEC;
			} else {
				;
			}
		} else if (iface->type == PFE_PHY_IF_HIF) {
			/*	HIF does not offer MAC filtering ability */
			;
		} else {
			NXP_LOG_ERROR("Invalid interface type\n");
			ret = EINVAL;
		}

		if (ret == EOK) {
			NXP_LOG_INFO(
				"Address %02x:%02x:%02x:%02x:%02x:%02x added to %s\n",
				addr[0], addr[1], addr[2], addr[3], addr[4],
				addr[5], iface->name);
		}
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Delete MAC address
 * @param[in]	iface The interface instance
 * @param[in]	addr The MAC address to delete
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 * @retval		ENOENT Address not found
 */
errno_t
pfe_phy_if_del_mac_addr(pfe_phy_if_t *iface, pfe_mac_addr_t addr)
{
	errno_t ret = EOK;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	/*	Configure also associated HW block */
	if (!iface->port.instance) {
		/*	No HW block associated */
		;
	} else {
		if (iface->type == PFE_PHY_IF_EMAC) {
			if (pfe_emac_del_addr(iface->port.emac, addr) != EOK) {
				ret = ENOENT;
			}
		} else if (iface->type == PFE_PHY_IF_HIF) {
			/*	HIF does not offer MAC filtering ability */
			;
		} else {
			NXP_LOG_ERROR("Invalid interface type\n");
			ret = EINVAL;
		}

		if (ret == EOK) {
			NXP_LOG_INFO(
				"Address %02x:%02x:%02x:%02x:%02x:%02x removed from %s\n",
				addr[0], addr[1], addr[2], addr[3], addr[4],
				addr[5], iface->name);
		}
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Get MAC address
 * @param[in]	iface The interface instance
 * @param[out]	addr The MAC address will be written here
 * @retval		EOK Success
 * @retval		EINVAL Invalid or missing argument
 * @retval		ENOENT No address found
 */
errno_t
pfe_phy_if_get_mac_addr(pfe_phy_if_t *iface, pfe_mac_addr_t addr)
{
	errno_t ret = EOK;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex lock failed\n");
	}

	/*	Get MAC address from associated HW block */
	if (!iface->port.instance) {
		/*	No HW block associated */
		;
	} else {
		if (iface->type == PFE_PHY_IF_EMAC) {
			ret = pfe_emac_get_addr(iface->port.emac, addr);
		} else if (iface->type == PFE_PHY_IF_HIF) {
			/*	HIF does not have MAC address storage (yet) */
			memset(addr, 0, sizeof(pfe_mac_addr_t));
		} else {
			/*	Unknown type, nothing to verify */
			;
		}
	}

	if (oal_mutex_unlock(&iface->lock) != EOK) {
		NXP_LOG_DEBUG("mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Get HW ID of the interface
 * @param[in]	iface The interface instance
 * @return		Interface ID
 */
__attribute__((pure)) pfe_ct_phy_if_id_t
pfe_phy_if_get_id(pfe_phy_if_t *iface)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return PFE_PHY_IF_ID_INVALID;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	return iface->id;
}

/**
 * @brief		Get name
 * @param[in]	iface The interface instance
 * @return		Pointer to interface name string or NULL if not found/failed
 */
__attribute__((pure)) char_t *
pfe_phy_if_get_name(pfe_phy_if_t *iface)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	return iface->name;
}

/**
 * @brief		Return physical interface runtime statistics in text form
 * @details		Function writes formatted text into given buffer.
 * @param[in]	iface 		The physical interface instance
 * @param[in]	buf 		A pointer to the buffer to write to
 * @param[in]	buf_len 	Buffer length
 * @param[in]	verb_level 	Verbosity level
 * @return		Number of bytes written to the buffer
 */
uint32_t
pfe_phy_if_get_text_statistics(pfe_phy_if_t *iface, char_t *buf,
			       u32 buf_len, uint8_t verb_level)
{
	uint32_t len = 0U;
	pfe_ct_phy_if_t phy_if_class = { 0U };
	uint32_t i;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!iface)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return 0U;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/* Repeat read for all PEs (just because of statistics) */
	for (i = 0U; i < pfe_class_get_num_of_pes(iface->class); i++) {
		/*
			Read current interface configuration from classifier. Since all class PEs are running the
			same code, also the data are the same (except statistics counters...).
			Returned data will be in __NETWORK__ endian format.
		*/
		if (EOK != pfe_class_read_dmem(iface->class, i, &phy_if_class,
					       (void *)iface->dmem_base,
					       sizeof(pfe_ct_phy_if_t))) {
			len += oal_util_snprintf(
				buf + len, buf_len - len,
				"[PhyIF 0x%x]: Unable to read DMEM\n",
				iface->id);
		} else {
			len += oal_util_snprintf(buf + len, buf_len - len,
						 "[PhyIF 0x%x '%s']\n",
						 iface->id,
						 pfe_phy_if_get_name(iface));
			len += oal_util_snprintf(
				buf + len, buf_len - len,
				"LogIfBase (DMEM) : 0x%x\n",
				oal_ntohl(phy_if_class.log_ifs));
			len += oal_util_snprintf(
				buf + len, buf_len - len,
				"DefLogIf  (DMEM) : 0x%x\n",
				oal_ntohl(phy_if_class.def_log_if));
			pfe_phy_if_stat_to_str(&phy_if_class.phy_stats,
					       buf + len, buf_len - len,
					       verb_level);
		}
	}
	return len;
}

/** @}*/
