/*
 * Store descriptor functions
 *
 * Copyright (C) 2011-2014, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include "libvshadow_block_descriptor.h"
#include "libvshadow_block_range_descriptor.h"
#include "libvshadow_block_tree.h"
#include "libvshadow_debug.h"
#include "libvshadow_definitions.h"
#include "libvshadow_io_handle.h"
#include "libvshadow_libbfio.h"
#include "libvshadow_libcdata.h"
#include "libvshadow_libcerror.h"
#include "libvshadow_libcnotify.h"
#include "libvshadow_libcstring.h"
#include "libvshadow_libcthreads.h"
#include "libvshadow_libfdatetime.h"
#include "libvshadow_libfguid.h"
#include "libvshadow_libuna.h"
#include "libvshadow_store_block.h"
#include "libvshadow_store_descriptor.h"

#include "vshadow_store.h"

/* Creates a store descriptor
 * ake sure the value store_descriptor is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_initialize(
     libvshadow_store_descriptor_t **store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_initialize";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( *store_descriptor != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid store descriptor value already set.",
		 function );

		return( -1 );
	}
	*store_descriptor = memory_allocate_structure(
	                     libvshadow_store_descriptor_t );

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create store descriptor.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *store_descriptor,
	     0,
	     sizeof( libvshadow_store_descriptor_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear store descriptor.",
		 function );

		memory_free(
		 *store_descriptor );

		*store_descriptor = NULL;

		return( -1 );
	}
	if( libcdata_list_initialize(
	     &( ( *store_descriptor )->block_descriptors_list ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create block descriptors list.",
		 function );

		goto on_error;
	}
	if( libcdata_btree_initialize(
	     &( ( *store_descriptor )->forward_block_descriptors_tree ),
	     LIBVSHADOW_BLOCK_DESCRIPTORS_TREE_MAXIMUM_NUMBER_OF_SUB_NODES,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create forward block descriptors tree.",
		 function );

		goto on_error;
	}
	if( libcdata_btree_initialize(
	     &( ( *store_descriptor )->reverse_block_descriptors_tree ),
	     LIBVSHADOW_BLOCK_DESCRIPTORS_TREE_MAXIMUM_NUMBER_OF_SUB_NODES,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create reverse block descriptors tree.",
		 function );

		goto on_error;
	}
	if( libcdata_range_list_initialize(
	     &( ( *store_descriptor )->block_offset_list ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create block offsets list.",
		 function );

		goto on_error;
	}
	if( libcdata_range_list_initialize(
	     &( ( *store_descriptor )->previous_block_offset_list ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create previous block offsets list.",
		 function );

		goto on_error;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_initialize(
	     &( ( *store_descriptor )->read_write_lock ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to intialize read/write lock.",
		 function );

		goto on_error;
	}
#endif
	return( 1 );

on_error:
	if( *store_descriptor != NULL )
	{
		if( ( *store_descriptor )->block_offset_list != NULL )
		{
			libcdata_range_list_free(
			 &( ( *store_descriptor )->block_offset_list ),
			 NULL,
			 NULL );
		}
		if( ( *store_descriptor )->reverse_block_descriptors_tree != NULL )
		{
			libcdata_btree_free(
			 &( ( *store_descriptor )->reverse_block_descriptors_tree ),
			 NULL,
			 NULL );
		}
		if( ( *store_descriptor )->forward_block_descriptors_tree != NULL )
		{
			libcdata_btree_free(
			 &( ( *store_descriptor )->forward_block_descriptors_tree ),
			 NULL,
			 NULL );
		}
		if( ( *store_descriptor )->block_descriptors_list != NULL )
		{
			libcdata_list_free(
			 &( ( *store_descriptor )->block_descriptors_list ),
			 NULL,
			 NULL );
		}
		memory_free(
		 *store_descriptor );

		*store_descriptor = NULL;
	}
	return( -1 );
}

/* Frees a store descriptor
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_free(
     libvshadow_store_descriptor_t **store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_free";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( *store_descriptor != NULL )
	{
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
		if( libcthreads_read_write_lock_free(
		     &( ( *store_descriptor )->read_write_lock ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free read/write lock.",
			 function );

			result = -1;
		}
#endif
		if( ( *store_descriptor )->operating_machine_string != NULL )
		{
			memory_free(
			 ( *store_descriptor )->operating_machine_string );
		}
		if( ( *store_descriptor )->service_machine_string != NULL )
		{
			memory_free(
			 ( *store_descriptor )->service_machine_string );
		}
		if( libcdata_btree_free(
		     &( ( *store_descriptor )->reverse_block_descriptors_tree ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libvshadow_block_descriptor_free_reverse,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free reverse block descriptors tree.",
			 function );

			result = -1;
		}
		if( libcdata_btree_free(
		     &( ( *store_descriptor )->forward_block_descriptors_tree ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libvshadow_block_descriptor_free,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free forward block descriptors tree.",
			 function );

			result = -1;
		}
		if( libcdata_list_free(
		     &( ( *store_descriptor )->block_descriptors_list ),
		     (int (*)(intptr_t **, libcerror_error_t **)) &libvshadow_block_descriptor_free,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free forward block descriptors tree.",
			 function );

			result = -1;
		}
		if( libcdata_range_list_free(
		     &( ( *store_descriptor )->block_offset_list ),
		     NULL,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free block offsets list.",
			 function );

			result = -1;
		}
		if( libcdata_range_list_free(
		     &( ( *store_descriptor )->previous_block_offset_list ),
		     NULL,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free previous block offsets list.",
			 function );

			result = -1;
		}
		memory_free(
		 *store_descriptor );

		*store_descriptor = NULL;
	}
	return( result );
}

/* Compares 2 store descriptors by their creation time
 * Returns LIBCDATA_COMPARE_LESS, LIBCDATA_COMPARE_EQUAL, LIBCDATA_COMPARE_GREATER if successful or -1 on error
 */
int libvshadow_store_descriptor_compare_by_creation_time(
     libvshadow_store_descriptor_t *first_store_descriptor,
     libvshadow_store_descriptor_t *second_store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_compare_by_creation_time";

	if( first_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid first store descriptor.",
		 function );

		return( -1 );
	}
	if( second_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid second store descriptor.",
		 function );

		return( -1 );
	}
	if( first_store_descriptor->creation_time < second_store_descriptor->creation_time )
	{
		return( LIBCDATA_COMPARE_LESS );
	}
	else if( first_store_descriptor->creation_time > second_store_descriptor->creation_time )
	{
		return( LIBCDATA_COMPARE_GREATER );
	}
	return( LIBCDATA_COMPARE_EQUAL );
}

/* Compares 2 store descriptors by their identifier
 * Returns LIBCDATA_COMPARE_LESS, LIBCDATA_COMPARE_EQUAL, LIBCDATA_COMPARE_GREATER if successful or -1 on error
 */
int libvshadow_store_descriptor_compare_by_identifier(
     libvshadow_store_descriptor_t *first_store_descriptor,
     libvshadow_store_descriptor_t *second_store_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_compare_by_identifier";
	int result            = 0;

	if( first_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid first store descriptor.",
		 function );

		return( -1 );
	}
	if( second_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid second store descriptor.",
		 function );

		return( -1 );
	}
	result = memory_compare(
	          first_store_descriptor->identifier,
	          second_store_descriptor->identifier,
	          16 );

	if( result < 0 )
	{
		return( LIBCDATA_COMPARE_LESS );
	}
	else if( result > 0 )
	{
		return( LIBCDATA_COMPARE_GREATER );
	}
	return( LIBCDATA_COMPARE_EQUAL );
}

/* Reads the catalog entry
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_catalog_entry(
     libvshadow_store_descriptor_t *store_descriptor,
     const uint8_t *catalog_block_data,
     size_t catalog_block_data_size,
     uint64_t *entry_type,
     libcerror_error_t **error )
{
	static char *function             = "libvshadow_store_descriptor_read_catalog_entry";

#if defined( HAVE_DEBUG_OUTPUT )
	libcstring_system_character_t filetime_string[ 32 ];
	libcstring_system_character_t guid_string[ 48 ];

	libfdatetime_filetime_t *filetime = NULL;
	libfguid_identifier_t *guid       = NULL;
	uint64_t value_64bit              = 0;
	int result                        = 0;
#endif

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( catalog_block_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid catalog block data.",
		 function );

		return( -1 );
	}
	if( catalog_block_data_size < 128 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid catalog block data size value too small.",
		 function );

		return( -1 );
	}
	if( catalog_block_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid catalog block data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( entry_type == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid entry type.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: catalog block entry data:\n",
		 function );
		libcnotify_print_data(
		 catalog_block_data,
		 128,
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif
	byte_stream_copy_to_uint64_little_endian(
	 catalog_block_data,
	 *entry_type );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: entry type\t\t\t: %" PRIu64 "\n",
		 function,
		 *entry_type );
	}
#endif
	if( ( *entry_type != 0 )
	 && ( *entry_type != 1 )
	 && ( *entry_type != 2 )
	 && ( *entry_type != 3 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported catalog entry type: %" PRIu64 ".",
		 function,
		 *entry_type );

		goto on_error;
	}
	if( ( *entry_type == 0 )
	 || ( *entry_type == 1 ) )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "\n" );
		}
#endif
/* TODO empty data check ? */
	}
	else if( *entry_type == 2 )
	{
		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 8 ] ),
		 store_descriptor->volume_size );

		if( memory_copy(
		     store_descriptor->identifier,
		     &( catalog_block_data[ 16 ] ),
		     16 ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy identifier.",
			 function );

			goto on_error;
		}
		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 48 ] ),
		 store_descriptor->creation_time );

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: volume size\t\t\t: %" PRIu64 "\n",
			 function,
			 store_descriptor->volume_size );

			if( libfguid_identifier_initialize(
			     &guid,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create GUID.",
				 function );

				goto on_error;
			}
			if( libfguid_identifier_copy_from_byte_stream(
			     guid,
			     &( catalog_block_data[ 16 ] ),
			     16,
			     LIBFGUID_ENDIAN_LITTLE,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy byte stream to GUID.",
				 function );

				goto on_error;
			}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfguid_identifier_copy_to_utf16_string(
				  guid,
				  (uint16_t *) guid_string,
				  48,
				  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
				  error );
#else
			result = libfguid_identifier_copy_to_utf8_string(
				  guid,
				  (uint8_t *) guid_string,
				  48,
				  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
				  error );
#endif
			if( result != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy GUID to string.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
			 "%s: store identifier\t\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
			 function,
			 guid_string );

			if( libfguid_identifier_free(
			     &guid,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free GUID.",
				 function );

				goto on_error;
			}
			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 32 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: unknown2\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 40 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: unknown3\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			if( libfdatetime_filetime_initialize(
			     &filetime,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create filetime.",
				 function );

				goto on_error;
			}
			if( libfdatetime_filetime_copy_from_byte_stream(
			     filetime,
			     &( catalog_block_data[ 48 ] ),
			     8,
			     LIBFDATETIME_ENDIAN_LITTLE,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to copy filetime from byte stream.",
				 function );

				goto on_error;
			}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfdatetime_filetime_copy_to_utf16_string(
				  filetime,
				  (uint16_t *) filetime_string,
				  32,
				  LIBFDATETIME_STRING_FORMAT_TYPE_CTIME | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
				  error );
#else
			result = libfdatetime_filetime_copy_to_utf8_string(
				  filetime,
				  (uint8_t *) filetime_string,
				  32,
				  LIBFDATETIME_STRING_FORMAT_TYPE_CTIME | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
				  error );
#endif
			if( result != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to copy filetime to string.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
			 "%s: creation time\t\t\t: %" PRIs_LIBCSTRING_SYSTEM " UTC\n",
			 function,
			 filetime_string );

			if( libfdatetime_filetime_free(
			     &filetime,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free filetime.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
			 "%s: unknown4:\n",
			 function );
			libcnotify_print_data(
			 &( catalog_block_data[ 56 ] ),
			 72,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
		}
#endif
	}
	else if( *entry_type == 3 )
	{
		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 8 ] ),
		 store_descriptor->store_block_list_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 32 ] ),
		 store_descriptor->store_header_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 40 ] ),
		 store_descriptor->store_block_range_list_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 48 ] ),
		 store_descriptor->store_bitmap_offset );

		byte_stream_copy_to_uint64_little_endian(
		 &( catalog_block_data[ 72 ] ),
		 store_descriptor->store_previous_bitmap_offset );

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store block list offset\t\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_block_list_offset );

			if( libfguid_identifier_initialize(
			     &guid,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create GUID.",
				 function );

				goto on_error;
			}
			if( libfguid_identifier_copy_from_byte_stream(
			     guid,
			     &( catalog_block_data[ 16 ] ),
			     16,
			     LIBFGUID_ENDIAN_LITTLE,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy byte stream to GUID.",
				 function );

				goto on_error;
			}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfguid_identifier_copy_to_utf16_string(
				  guid,
				  (uint16_t *) guid_string,
				  48,
				  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
				  error );
#else
			result = libfguid_identifier_copy_to_utf8_string(
				  guid,
				  (uint8_t *) guid_string,
				  48,
				  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
				  error );
#endif
			if( result != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy GUID to string.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
			 "%s: store identifier\t\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
			 function,
			 guid_string );

			if( libfguid_identifier_free(
			     &guid,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free GUID.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
			 "%s: store header offset\t\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_header_offset );

			libcnotify_printf(
			 "%s: store block range list offset\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_block_range_list_offset );

			libcnotify_printf(
			 "%s: store bitmap offset\t\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_bitmap_offset );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 56 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: store file reference\t\t: MFT entry: %" PRIu64 ", sequence: %" PRIu64 "\n",
			 function,
			 value_64bit & 0xffffffffffffUL,
			 value_64bit >> 48 );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 64 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: allocated size\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			libcnotify_printf(
			 "%s: store previous bitmap offset\t: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->store_previous_bitmap_offset );

			byte_stream_copy_to_uint64_little_endian(
			 &( catalog_block_data[ 80 ] ),
			 value_64bit );
			libcnotify_printf(
			 "%s: unknown2\t\t\t: %" PRIu64 "\n",
			 function,
			 value_64bit );

			libcnotify_printf(
			 "%s: unknown3:\n",
			 function );
			libcnotify_print_data(
			 &( catalog_block_data[ 88 ] ),
			 40,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
		}
#endif
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );

on_error:
#if defined( HAVE_DEBUG_OUTPUT )
	if( filetime != NULL )
	{
		libfdatetime_filetime_free(
		 &filetime,
		 NULL );
	}
	if( guid != NULL )
	{
		libfguid_identifier_free(
		 &guid,
		 NULL );
	}
#endif
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_write(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Reads the store header
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_header(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block       = NULL;
	uint8_t *store_header_data                  = NULL;
	static char *function                       = "libvshadow_store_descriptor_read_store_header";
	size_t store_header_data_offset             = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	libcstring_system_character_t guid_string[ 48 ];

	libcstring_system_character_t *value_string = NULL;
	libfguid_identifier_t *guid                 = NULL;
	size_t value_string_size                    = 0;
	uint32_t value_32bit                        = 0;
	int result                                  = 0;
#endif

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     store_descriptor->store_header_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 store_descriptor->store_header_offset );

		goto on_error;
	}
	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_HEADER )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		return( -1 );
	}
	store_header_data = &( store_block->data[ sizeof( vshadow_store_header_t ) ] );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: store header information:\n",
		 function );
		libcnotify_print_data(
		 store_header_data,
		 sizeof( vshadow_store_information_t ),
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif
	if( memory_copy(
	     store_descriptor->copy_identifier,
	     ( (vshadow_store_information_t *) store_header_data )->copy_identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy shadow copy identifier.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     store_descriptor->copy_set_identifier,
	     ( (vshadow_store_information_t *) store_header_data )->copy_set_identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy shadow copy set identifier.",
		 function );

		goto on_error;
	}
	byte_stream_copy_to_uint32_little_endian(
	 ( (vshadow_store_information_t *) store_header_data )->attribute_flags,
	 store_descriptor->attribute_flags );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( libfguid_identifier_initialize(
		     &guid,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create GUID.",
			 function );

			goto on_error;
		}
		if( libfguid_identifier_copy_from_byte_stream(
		     guid,
		     ( (vshadow_store_information_t *) store_header_data )->unknown5,
		     16,
		     LIBFGUID_ENDIAN_LITTLE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy byte stream to GUID.",
			 function );

			goto on_error;
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfguid_identifier_copy_to_utf16_string(
			  guid,
			  (uint16_t *) guid_string,
			  48,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			  error );
#else
		result = libfguid_identifier_copy_to_utf8_string(
			  guid,
			  (uint8_t *) guid_string,
			  48,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy GUID to string.",
			 function );

			goto on_error;
		}
		libcnotify_printf(
		 "%s: unknown5\t\t\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
		 function,
		 guid_string );

		if( libfguid_identifier_copy_from_byte_stream(
		     guid,
		     ( (vshadow_store_information_t *) store_header_data )->copy_identifier,
		     16,
		     LIBFGUID_ENDIAN_LITTLE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy byte stream to GUID.",
			 function );

			goto on_error;
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfguid_identifier_copy_to_utf16_string(
			  guid,
			  (uint16_t *) guid_string,
			  48,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			  error );
#else
		result = libfguid_identifier_copy_to_utf8_string(
			  guid,
			  (uint8_t *) guid_string,
			  48,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy GUID to string.",
			 function );

			goto on_error;
		}
		libcnotify_printf(
		 "%s: copy identifier\t\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
		 function,
		 guid_string );

		if( libfguid_identifier_copy_from_byte_stream(
		     guid,
		     ( (vshadow_store_information_t *) store_header_data )->copy_set_identifier,
		     16,
		     LIBFGUID_ENDIAN_LITTLE,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy byte stream to GUID.",
			 function );

			goto on_error;
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libfguid_identifier_copy_to_utf16_string(
			  guid,
			  (uint16_t *) guid_string,
			  48,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			  error );
#else
		result = libfguid_identifier_copy_to_utf8_string(
			  guid,
			  (uint8_t *) guid_string,
			  48,
			  LIBFGUID_STRING_FORMAT_FLAG_USE_LOWER_CASE,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy GUID to string.",
			 function );

			goto on_error;
		}
		libcnotify_printf(
		 "%s: copy set identifier\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
		 function,
		 guid_string );

		if( libfguid_identifier_free(
		     &guid,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free GUID.",
			 function );

			goto on_error;
		}
		byte_stream_copy_to_uint32_little_endian(
		 ( (vshadow_store_information_t *) store_header_data )->type,
		 value_32bit );
		libcnotify_printf(
		 "%s: type\t\t\t: 0x%08" PRIx32 "\n",
		 function,
		 value_32bit );

		byte_stream_copy_to_uint32_little_endian(
		 ( (vshadow_store_information_t *) store_header_data )->provider,
		 value_32bit );
		libcnotify_printf(
		 "%s: provider\t\t\t: %" PRIu32 "\n",
		 function,
		 value_32bit );

		libcnotify_printf(
		 "%s: attribute flags\t\t: 0x%08" PRIx32 "\n",
		 function,
		 store_descriptor->attribute_flags );
		libvshadow_debug_print_attribute_flags(
		 store_descriptor->attribute_flags );
		libcnotify_printf(
		 "\n" );

		byte_stream_copy_to_uint32_little_endian(
		 ( (vshadow_store_information_t *) store_header_data )->unknown10,
		 value_32bit );
		libcnotify_printf(
		 "%s: unknown10\t\t: 0x%08" PRIx32 "\n",
		 function,
		 value_32bit );
	}
#endif
	store_header_data_offset = sizeof( vshadow_store_information_t );

	byte_stream_copy_to_uint16_little_endian(
	 &( store_header_data[ store_header_data_offset ] ),
	 store_descriptor->operating_machine_string_size );

	store_header_data_offset += 2;

	if( ( store_header_data_offset + store_descriptor->operating_machine_string_size ) > store_block->data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: operating machine string size value out of bounds.",
		 function );

		goto on_error;
	}
	store_descriptor->operating_machine_string = (uint8_t *) memory_allocate(
	                                                          sizeof( uint8_t ) * store_descriptor->operating_machine_string_size );

	if( store_descriptor->operating_machine_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create operating machine string.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     store_descriptor->operating_machine_string,
	     &( store_header_data[ store_header_data_offset ] ),
	     (size_t) store_descriptor->operating_machine_string_size ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy operating machine string.",
		 function );

		goto on_error;
	}
	store_header_data_offset += store_descriptor->operating_machine_string_size;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libuna_utf16_string_size_from_utf16_stream(
			  store_descriptor->operating_machine_string,
			  (size_t) store_descriptor->operating_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  &value_string_size,
			  error );
#else
		result = libuna_utf8_string_size_from_utf16_stream(
			  store_descriptor->operating_machine_string,
			  (size_t) store_descriptor->operating_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  &value_string_size,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine value string size.",
			 function );

			goto on_error;
		}
		value_string = libcstring_system_string_allocate(
		                value_string_size );

		if( value_string == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create value string.",
			 function );

			goto on_error;
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libuna_utf16_string_copy_from_utf16_stream(
			  (libuna_utf16_character_t *) value_string,
			  value_string_size,
			  store_descriptor->operating_machine_string,
			  (size_t) store_descriptor->operating_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  error );
#else
		result = libuna_utf8_string_copy_from_utf16_stream(
			  (libuna_utf8_character_t *) value_string,
			  value_string_size,
			  store_descriptor->operating_machine_string,
			  (size_t) store_descriptor->operating_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  error );
#endif

		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set value string.",
			 function );

			memory_free(
			 value_string );

			goto on_error;
		}
		libcnotify_printf(
		 "%s: operating machine string\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
		 function,
		 value_string );

		memory_free(
		 value_string );
	}
#endif
	byte_stream_copy_to_uint16_little_endian(
	 &( store_header_data[ store_header_data_offset ] ),
	 store_descriptor->service_machine_string_size );

	store_header_data_offset += 2;

	if( ( store_header_data_offset + store_descriptor->service_machine_string_size ) > store_block->data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: operating machine string size value out of bounds.",
		 function );

		goto on_error;
	}
	store_descriptor->service_machine_string = (uint8_t *) memory_allocate(
	                                                        sizeof( uint8_t ) * store_descriptor->service_machine_string_size );

	if( store_descriptor->service_machine_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create service machine string.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     store_descriptor->service_machine_string,
	     &( store_header_data[ store_header_data_offset ] ),
	     (size_t) store_descriptor->service_machine_string_size ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy service machine string.",
		 function );

		goto on_error;
	}
	store_header_data_offset += store_descriptor->service_machine_string_size;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libuna_utf16_string_size_from_utf16_stream(
			  store_descriptor->service_machine_string,
			  (size_t) store_descriptor->service_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  &value_string_size,
			  error );
#else
		result = libuna_utf8_string_size_from_utf16_stream(
			  store_descriptor->service_machine_string,
			  (size_t) store_descriptor->service_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  &value_string_size,
			  error );
#endif
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine value string size.",
			 function );

			goto on_error;
		}
		value_string = libcstring_system_string_allocate(
		                value_string_size );

		if( value_string == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create value string.",
			 function );

			goto on_error;
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libuna_utf16_string_copy_from_utf16_stream(
			  (libuna_utf16_character_t *) value_string,
			  value_string_size,
			  store_descriptor->service_machine_string,
			  (size_t) store_descriptor->service_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  error );
#else
		result = libuna_utf8_string_copy_from_utf16_stream(
			  (libuna_utf8_character_t *) value_string,
			  value_string_size,
			  store_descriptor->service_machine_string,
			  (size_t) store_descriptor->service_machine_string_size,
			  LIBUNA_ENDIAN_LITTLE,
			  error );
#endif

		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set value string.",
			 function );

			memory_free(
			 value_string );

			goto on_error;
		}
		libcnotify_printf(
		 "%s: service machine string\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
		 function,
		 value_string );

		memory_free(
		 value_string );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( store_header_data_offset < store_block->data_size )
		{
			libcnotify_printf(
			 "%s: trailing data:\n",
			 function );
			libcnotify_print_data(
			 &( store_header_data[ store_header_data_offset ] ),
			 store_block->data_size - store_header_data_offset,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
		}
	}
#endif
	if( libvshadow_store_block_free(
	     &store_block,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free store block.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
#endif
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );

on_error:
	if( store_block != NULL )
	{
		libvshadow_store_block_free(
		 &store_block,
		 NULL );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_write(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Reads the store bitmap
 * This function is not multi-thread safe acquire write lock before call
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_bitmap(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     off64_t file_offset,
     libcdata_range_list_t *offset_list,
     off64_t *bitmap_offset,
     off64_t *next_offset,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block = NULL;
	uint8_t *block_data                   = NULL;
	static char *function                 = "libvshadow_store_descriptor_read_store_bitmap";
	off64_t start_offset                  = 0;
	uint32_t value_32bit                  = 0;
	uint16_t block_size                   = 0;
	uint8_t bit_index                     = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( bitmap_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid bitmap offset.",
		 function );

		return( -1 );
	}
	if( next_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid next offset.",
		 function );

		return( -1 );
	}
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     file_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 file_offset );

		goto on_error;
	}
	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_BITMAP )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		goto on_error;
	}
	*next_offset = store_block->next_offset;

	block_data = &( store_block->data[ sizeof( vshadow_store_block_header_t ) ] );
	block_size = (uint16_t) ( store_block->data_size - sizeof( vshadow_store_block_header_t ) );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: store: %02d block bitmap:\n",
		 function,
		 store_descriptor->index );
		libcnotify_print_data(
		 block_data,
		 block_size,
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif
	start_offset = -1;

	while( block_size > 0 )
	{
		byte_stream_copy_to_uint32_little_endian(
		 block_data,
		 value_32bit );

		for( bit_index = 0;
		     bit_index < 32;
		     bit_index++ )
		{
			if( ( value_32bit & 0x00000001UL ) == 0 )
			{
				if( start_offset >= 0 )
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: store: %02d offset range\t: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
						 function,
						 store_descriptor->index,
						 start_offset,
						 *bitmap_offset,
						 *bitmap_offset - start_offset );
					}
#endif
					if( libcdata_range_list_insert_range(
					     offset_list,
					     (uint64_t) start_offset,
					     (uint64_t) ( *bitmap_offset - start_offset ),
					     NULL,
					     NULL,
					     NULL,
					     error ) != 1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
						 "%s: unable to insert offset range to offset list.",
						 function );

						goto on_error;
					}
					start_offset = -1;
				}
			}
			else
			{
				if( start_offset < 0 )
				{
					start_offset = *bitmap_offset;
				}
			}
			*bitmap_offset += 0x4000;

			value_32bit >>= 1;
		}
		block_data += 4;
		block_size -= 4;
	}
	if( start_offset >= 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store: %02d offset range\t: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
			 function,
			 store_descriptor->index,
			 start_offset,
			 *bitmap_offset,
			 *bitmap_offset - start_offset );
		}
#endif
		if( libcdata_range_list_insert_range(
		     offset_list,
		     (uint64_t) start_offset,
		     (uint64_t) ( *bitmap_offset - start_offset ),
		     NULL,
		     NULL,
		     NULL,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
			 "%s: unable to append offset range to offset list.",
			 function );

			goto on_error;
		}
	}
	if( libvshadow_store_block_free(
	     &store_block,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free store block.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
#endif
	return( 1 );

on_error:
	if( store_block != NULL )
	{
		libvshadow_store_block_free(
		 &store_block,
		 NULL );
	}
	return( -1 );
}

/* Reads the store block list
 * This function is not multi-thread safe acquire write lock before call
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_block_list(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     off64_t file_offset,
     off64_t *next_offset,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block           = NULL;
	libvshadow_block_descriptor_t *block_descriptor = NULL;
	uint8_t *block_data                             = NULL;
	static char *function                           = "libvshadow_store_descriptor_read_store_block_list";
	uint16_t block_size                             = 0;
	int result                                      = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( next_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid next offset.",
		 function );

		return( -1 );
	}
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     file_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 file_offset );

		goto on_error;
	}
	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_INDEX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		goto on_error;
	}
	*next_offset = store_block->next_offset;

	block_data = &( store_block->data[ sizeof( vshadow_store_block_header_t ) ] );
	block_size = (uint16_t) ( store_block->data_size - sizeof( vshadow_store_block_header_t ) );

	while( block_size >= sizeof( vshadow_store_block_list_entry_t ) )
	{
		if( libvshadow_block_descriptor_initialize(
		     &block_descriptor,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create block descriptor.",
			 function );

			goto on_error;
		}
		result = libvshadow_block_descriptor_read(
			  block_descriptor,
			  block_data,
			  block_size,
			  store_descriptor->index,
			  error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read block descriptor.",
			 function );

			goto on_error;
		}
		else if( result != 0 )
		{
			if( libvshadow_block_tree_insert(
			     store_descriptor->forward_block_descriptors_tree,
			     store_descriptor->reverse_block_descriptors_tree,
			     block_descriptor,
			     store_descriptor->index,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to insert block descriptor in tree.",
				 function );

				goto on_error;
			}
			if( libcdata_list_append_value(
			     store_descriptor->block_descriptors_list,
			     (intptr_t *) block_descriptor,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to append block descriptor to list.",
				 function );

				goto on_error;
			}
			block_descriptor = NULL;
		}
		if( block_descriptor != NULL )
		{
			if( libvshadow_block_descriptor_free(
			     &block_descriptor,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free block descriptor.",
				 function );

				block_descriptor = NULL;

				goto on_error;
			}
			block_descriptor = NULL;
		}
		block_data += sizeof( vshadow_store_block_list_entry_t );
		block_size -= sizeof( vshadow_store_block_list_entry_t );
	}
	if( libvshadow_store_block_free(
	     &store_block,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free store block.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( block_descriptor != NULL )
	{
		libvshadow_block_descriptor_free(
		 &block_descriptor,
		 NULL );
	}
	if( store_block != NULL )
	{
		libvshadow_store_block_free(
		 &store_block,
		 NULL );
	}
	return( -1 );
}

/* Reads the store block range list
 * This function is not multi-thread safe acquire write lock before call
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_store_block_range_list(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     off64_t file_offset,
     off64_t *next_offset,
     libcerror_error_t **error )
{
	libvshadow_store_block_t *store_block                       = NULL;
	libvshadow_block_range_descriptor_t *block_range_descriptor = NULL;
	uint8_t *block_data                                         = NULL;
	static char *function                                       = "libvshadow_store_descriptor_read_store_block_range_list";
	uint16_t block_size                                         = 0;
	int result                                                  = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( next_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid next offset.",
		 function );

		return( -1 );
	}
	if( libvshadow_store_block_initialize(
	     &store_block,
	     0x4000,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create store block.",
		 function );

		goto on_error;
	}
	if( libvshadow_store_block_read(
	     store_block,
	     file_io_handle,
	     file_offset,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read store block at offset: %" PRIi64 ".",
		 function,
		 file_offset );

		goto on_error;
	}
	if( store_block->record_type != LIBVSHADOW_RECORD_TYPE_STORE_BLOCK_RANGE )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported record type: %" PRIu32 ".",
		 function,
		 store_block->record_type );

		goto on_error;
	}
	*next_offset = store_block->next_offset;

	block_data = &( store_block->data[ sizeof( vshadow_store_block_header_t ) ] );
	block_size = (uint16_t) ( store_block->data_size - sizeof( vshadow_store_block_header_t ) );

	while( block_size >= sizeof( vshadow_store_block_list_entry_t ) )
	{
		if( libvshadow_block_range_descriptor_initialize(
		     &block_range_descriptor,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create block range descriptor.",
			 function );

			goto on_error;
		}
		result = libvshadow_block_range_descriptor_read(
			  block_range_descriptor,
			  block_data,
			  block_size,
			  store_descriptor->index,
			  error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read block range descriptor.",
			 function );

			goto on_error;
		}
		else if( result != 0 )
		{
/* TODO do something useful with the range descriptors */
		}
		if( block_range_descriptor != NULL )
		{
			if( libvshadow_block_range_descriptor_free(
			     &block_range_descriptor,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free block range descriptor.",
				 function );

				block_range_descriptor = NULL;

				goto on_error;
			}
			block_range_descriptor = NULL;
		}
		block_data += sizeof( vshadow_store_block_range_list_entry_t );
		block_size -= sizeof( vshadow_store_block_range_list_entry_t );
	}
	if( libvshadow_store_block_free(
	     &store_block,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free store block.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( block_range_descriptor != NULL )
	{
		libvshadow_block_range_descriptor_free(
		 &block_range_descriptor,
		 NULL );
	}
	if( store_block != NULL )
	{
		libvshadow_store_block_free(
		 &store_block,
		 NULL );
	}
	return( -1 );
}

/* Reads the block descriptors
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_read_block_descriptors(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     libcerror_error_t **error )
{
	static char *function      = "libvshadow_store_descriptor_read_block_descriptors";
	off64_t bitmap_offset      = 0;
	off64_t store_block_offset = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( store_descriptor->block_descriptors_read == 0 )
	{
		bitmap_offset      = 0;
		store_block_offset = store_descriptor->store_bitmap_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_bitmap(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     store_descriptor->block_offset_list,
			     &bitmap_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store bitmap.",
				 function );

				goto on_error;
			}
		}
		bitmap_offset      = 0;
		store_block_offset = store_descriptor->store_previous_bitmap_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_bitmap(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     store_descriptor->previous_block_offset_list,
			     &bitmap_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store previous bitmap.",
				 function );

				goto on_error;
			}
		}
		store_block_offset = store_descriptor->store_block_list_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_block_list(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store block list.",
				 function );

				goto on_error;
			}
		}
		store_block_offset = store_descriptor->store_block_range_list_offset;

		while( store_block_offset != 0 )
		{
			if( libvshadow_store_descriptor_read_store_block_range_list(
			     store_descriptor,
			     file_io_handle,
			     store_block_offset,
			     &store_block_offset,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read store block range list.",
				 function );

				goto on_error;
			}
		}
		store_descriptor->block_descriptors_read = 1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );

on_error:
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_write(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Reads data at the specified offset into a buffer
 * Returns the number of bytes read or -1 on error
 */
ssize_t libvshadow_store_descriptor_read_buffer(
         libvshadow_store_descriptor_t *store_descriptor,
         libbfio_handle_t *file_io_handle,
         uint8_t *buffer,
         size_t buffer_size,
         off64_t offset,
         libvshadow_store_descriptor_t *active_store_descriptor,
         libcerror_error_t **error )
{
	libcdata_tree_node_t *tree_node                         = NULL;
	libvshadow_block_descriptor_t *block_descriptor         = NULL;
	libvshadow_block_descriptor_t *overlay_block_descriptor = NULL;
	libvshadow_block_descriptor_t *reverse_block_descriptor = NULL;
	static char *function                                   = "libvshadow_store_descriptor_read_buffer";
	intptr_t *value                                         = NULL;
	off64_t block_descriptor_offset                         = 0;
	off64_t block_offset                                    = 0;
	off64_t block_range_offset                              = 0;
	off64_t overlay_block_offset                            = 0;
	off64_t previous_block_range_offset                     = 0;
	size64_t block_range_size                               = 0;
	size64_t previous_block_range_size                      = 0;
	size_t buffer_offset                                    = 0;
	size_t block_size                                       = 0;
	size_t read_size                                        = 0;
	ssize_t read_count                                      = 0;
	uint32_t overlay_bitmap                                 = 0;
	uint32_t relative_block_offset                          = 0;
	uint8_t bit_count                                       = 0;
	int in_block_descriptor_list                            = 0;
	int in_current_bitmap                                   = 0;
	int in_previous_bitmap                                  = 0;
	int in_reverse_block_descriptor_list                    = 0;
	int result                                              = 0;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( active_store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid active store descriptor.",
		 function );

		return( -1 );
	}
	/* This function will acquire the write lock
	 */
	if( libvshadow_store_descriptor_read_block_descriptors(
	     store_descriptor,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read block descriptors.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: store: %02d requested offset: 0x%08" PRIx64 "\n",
		 function,
		 store_descriptor->index,
		 offset );
	}
#endif
	while( buffer_size > 0 )
	{
		block_offset          = offset;
		relative_block_offset = (uint32_t) (offset % 0x4000);
		block_size            = 0x4000 - relative_block_offset;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store: %02d block offset: 0x%08" PRIx64 "\n",
			 function,
			 store_descriptor->index,
			 block_offset );
		}
#endif
		in_block_descriptor_list         = 0;
		in_reverse_block_descriptor_list = 0;
		in_current_bitmap                = 0;
		in_previous_bitmap               = 0;

		result = libcdata_btree_get_value_by_value(
		          store_descriptor->forward_block_descriptors_tree,
		          (intptr_t *) &block_offset,
		          (int (*)(intptr_t *, intptr_t *, libcerror_error_t **)) &libvshadow_block_descriptor_compare_range_by_original_offset_value,
		          &tree_node,
		          (intptr_t **) &block_descriptor,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve block range for offset: 0x%08" PRIx64 ".",
			 function,
			 block_offset );

			goto on_error;
		}
		else if( result != 0 )
		{
			if( block_descriptor == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				 "%s: missing block descriptor.",
				 function );

				goto on_error;
			}
			in_block_descriptor_list = 1;

			if( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_FORWARDER ) != 0 )
			{
				block_descriptor_offset = block_descriptor->relative_offset;
			}
			else
			{
				block_descriptor_offset = block_descriptor->offset;
			}
			if( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_OVERLAY ) != 0 )
			{
				overlay_block_descriptor = block_descriptor;
			}
			else
			{
				overlay_block_descriptor = block_descriptor->overlay;
			}
			if( overlay_block_descriptor != NULL )
			{
				if( store_descriptor->index != active_store_descriptor->index )
				{
					if( block_descriptor == overlay_block_descriptor )
					{
						block_descriptor         = NULL;
						in_block_descriptor_list = 0;
					}
				}
				else
				{
					overlay_block_offset = overlay_block_descriptor->original_offset;
					overlay_bitmap       = overlay_block_descriptor->bitmap;

					bit_count = 32;

					while( overlay_block_offset < block_offset )
					{
						overlay_bitmap >>= 1;

						overlay_block_offset += 512;

						bit_count--;

						if( bit_count == 0 )
						{
							break;
						}
					}
					if( ( overlay_bitmap & 0x00000001UL ) != 0 )
					{
						block_descriptor_offset = overlay_block_descriptor->offset;
						block_descriptor        = overlay_block_descriptor;

						block_size = 0;

						while( ( overlay_bitmap & 0x00000001UL ) != 0 )
						{
							overlay_bitmap >>= 1;

							block_size += 512;

							bit_count--;

							if( bit_count == 0 )
							{
								break;
							}
						}
					}
					else
					{
						if( block_descriptor == overlay_block_descriptor )
						{
							block_descriptor         = NULL;
							in_block_descriptor_list = 0;
						}
						block_size = 0;

						while( ( overlay_bitmap & 0x00000001UL ) == 0 )
						{
							overlay_bitmap >>= 1;

							block_size += 512;

							bit_count--;

							if( bit_count == 0 )
							{
								break;
							}
						}
					}
				}
			}
		}
		if( in_block_descriptor_list == 0 )
		{
			/* Only the most recent store seems to bother checking the current bitmap
			 */
			if( ( store_descriptor->next_store_descriptor == NULL )
			 && ( store_descriptor->index == active_store_descriptor->index ) )
			{
				if( store_descriptor->reverse_block_descriptors_tree == NULL )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
					 "%s: invalid store descriptor - missing reverse block descriptors tree.",
					 function );

					goto on_error;
				}
				result = libcdata_btree_get_value_by_value(
					  store_descriptor->reverse_block_descriptors_tree,
					  (intptr_t *) &block_offset,
					  (int (*)(intptr_t *, intptr_t *, libcerror_error_t **)) &libvshadow_block_descriptor_compare_range_by_relative_offset_value,
					  &tree_node,
					  (intptr_t **) &reverse_block_descriptor,
					  error );

				if( result == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve reverse block descriptor from root node.",
					 function );

					goto on_error;
				}
				in_reverse_block_descriptor_list = result;

				result = libcdata_range_list_get_range_at_offset(
					  store_descriptor->block_offset_list,
					  (uint64_t) block_offset,
					  (uint64_t *) &block_range_offset,
					  (uint64_t *) &block_range_size,
					  &value,
					  error );

				if( result == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve offset: 0x%08" PRIx64 " from block offset list.",
					 function,
					 offset );

					goto on_error;
				}
#if defined( HAVE_DEBUG_OUTPUT )
				else if( result != 0 )
				{
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: store: %02d block offset list: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
						 function,
						 store_descriptor->index,
						 block_range_offset,
						 block_range_offset + block_range_size,
						 block_range_size );
					}
				}
#endif
				in_current_bitmap = result;

				if( store_descriptor->store_previous_bitmap_offset != 0 )
				{
					result = libcdata_range_list_get_range_at_offset(
						  store_descriptor->previous_block_offset_list,
						  (uint64_t) block_offset,
						  (uint64_t *) &previous_block_range_offset,
						  (uint64_t *) &previous_block_range_size,
						  &value,
						  error );

					if( result == -1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						 "%s: unable to retrieve offset: 0x%08" PRIx64 " from previous block offset list.",
						 function,
						 offset );

						goto on_error;
					}
#if defined( HAVE_DEBUG_OUTPUT )
					else if( result != 0 )
					{
						if( libcnotify_verbose != 0 )
						{
							libcnotify_printf(
							 "%s: store: %02d previous block offset list: 0x%08" PRIx64 " - 0x%08" PRIx64 " (0x%08" PRIx64 ")\n",
							 function,
							 store_descriptor->index,
							 previous_block_range_offset,
							 previous_block_range_offset + previous_block_range_size,
							 previous_block_range_size );
						}
					}
#endif
					in_previous_bitmap = result;
				}
				else
				{
					in_previous_bitmap = 1;
				}
			}
		}
		block_descriptor_offset += relative_block_offset;

		if( buffer_size > block_size )
		{
			read_size = block_size;
		}
		else
		{
			read_size = buffer_size;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: store: %02d range: 0x%08" PRIx64 " - 0x%08" PRIx64 " size: %" PRIzd "",
			 function,
			 store_descriptor->index,
			 block_offset,
			 block_offset + block_size,
			 block_size );

			if( block_descriptor != NULL )
			{
				libcnotify_printf(
				 ", flags: 0x%08" PRIx32 "",
				 block_descriptor->flags );
			}
			libcnotify_printf(
			 "\n" );

			if( in_block_descriptor_list != 0 )
			{
				libcnotify_printf(
				 "\tIn block list\n" );
			}
			if( block_descriptor != NULL )
			{
				if( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_OVERLAY ) != 0 )
				{
					libcnotify_printf(
					 "\tIs overlay\n" );
				}
			}
			if( in_reverse_block_descriptor_list != 0 )
			{
				libcnotify_printf(
				 "\tIn reverse block list\n" );
			}
			if( in_current_bitmap != 0 )
			{
				libcnotify_printf(
				 "\tIn current bitmap\n" );
			}
			if( ( store_descriptor->store_previous_bitmap_offset != 0 )
			 && ( in_previous_bitmap != 0 ) )
			{
				libcnotify_printf(
				 "\tIn previous bitmap\n" );
			}
			libcnotify_printf(
			 "\n" );
		}
#endif
		if( in_block_descriptor_list != 0 )
		{
			if( ( ( block_descriptor->flags & LIBVSHADOW_BLOCK_FLAG_IS_FORWARDER ) != 0 )
			 && ( store_descriptor->next_store_descriptor != NULL ) )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from next store at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_descriptor_offset );
				}
#endif
				read_count = libvshadow_store_descriptor_read_buffer(
					      store_descriptor->next_store_descriptor,
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      block_descriptor_offset,
					      active_store_descriptor,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from next store descriptor.",
					 function );

					goto on_error;
				}
			}
			else
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from current volume at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_descriptor_offset );
				}
#endif
				if( libbfio_handle_seek_offset(
				     file_io_handle,
				     block_descriptor_offset,
				     SEEK_SET,
				     error ) == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_SEEK_FAILED,
					 "%s: unable to seek block offset: %" PRIi64 ".",
					 function,
					 block_descriptor_offset );

					goto on_error;
				}
				read_count = libbfio_handle_read_buffer(
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from file IO handle.",
					 function );

					goto on_error;
				}
			}
		}
		else
		{
			/* Check if the next store defines the block
			 */
			if( store_descriptor->next_store_descriptor != NULL )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from next store at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_offset );
				}
#endif
				read_count = libvshadow_store_descriptor_read_buffer(
					      store_descriptor->next_store_descriptor,
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      block_offset,
					      active_store_descriptor,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from next store descriptor.",
					 function );

					goto on_error;
				}
			}
			else if( ( in_reverse_block_descriptor_list == 0 )
			      && ( in_current_bitmap != 0 )
			      && ( in_previous_bitmap != 0 ) )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d filling block with zero bytes\n",
					 function,
					 store_descriptor->index,
					 block_offset,
					 read_size );
				}
#endif
				if( memory_set(
				     &( buffer[ buffer_offset ] ),
				     0,
				     read_size ) == NULL )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_MEMORY,
					 LIBCERROR_MEMORY_ERROR_SET_FAILED,
					 "%s: unable to clear buffer.",
					 function );

					goto on_error;
				}
				read_count = (ssize_t) read_size;
			}
			else
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: store: %02d reading block from current volume at offset: 0x%08" PRIx64 "\n",
					 function,
					 store_descriptor->index,
					 block_offset );
				}
#endif
				if( libbfio_handle_seek_offset(
				     file_io_handle,
				     block_offset,
				     SEEK_SET,
				     error ) == -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_SEEK_FAILED,
					 "%s: unable to seek block offset: %" PRIi64 ".",
					 function,
					 block_offset );

					goto on_error;
				}
				read_count = libbfio_handle_read_buffer(
					      file_io_handle,
					      &( buffer[ buffer_offset ] ),
					      read_size,
					      error );

				if( read_count != (ssize_t) read_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read buffer from file IO handle.",
					 function );

					goto on_error;
				}
			}
		}
		offset        += read_count;
		buffer_offset += read_count;
		buffer_size   -= read_count;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "\n" );
		}
#endif
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( (ssize_t) buffer_offset );

on_error:
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	libcthreads_read_write_lock_release_for_read(
	 store_descriptor->read_write_lock,
	 NULL );
#endif
	return( -1 );
}

/* Retrieves the volume size
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_volume_size(
     libvshadow_store_descriptor_t *store_descriptor,
     size64_t *volume_size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_volume_size";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( volume_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume size.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	*volume_size = store_descriptor->volume_size;

#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );
}

/* Retrieves the identifier
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_identifier(
     libvshadow_store_descriptor_t *store_descriptor,
     uint8_t *guid,
     size_t size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_identifier";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( guid == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.",
		 function );

		return( -1 );
	}
	if( size < 16 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid GUID value too small.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( memory_copy(
	     guid,
	     store_descriptor->identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy identifier.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the creation date and time
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_creation_time(
     libvshadow_store_descriptor_t *store_descriptor,
     uint64_t *filetime,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_creation_time";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( filetime == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filetime.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	*filetime = store_descriptor->creation_time;

#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );
}

/* Retrieves the copy identifier
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_copy_identifier(
     libvshadow_store_descriptor_t *store_descriptor,
     uint8_t *guid,
     size_t size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_copy_identifier";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( guid == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.",
		 function );

		return( -1 );
	}
	if( size < 16 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid GUID value too small.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( memory_copy(
	     guid,
	     store_descriptor->copy_identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy shadow copy identifier.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the copy set identifier
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_copy_set_identifier(
     libvshadow_store_descriptor_t *store_descriptor,
     uint8_t *guid,
     size_t size,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_copy_set_identifier";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( guid == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.",
		 function );

		return( -1 );
	}
	if( size < 16 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid GUID value too small.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( memory_copy(
	     guid,
	     store_descriptor->copy_set_identifier,
	     16 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy shadow copy set identifier.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the attribute flags
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_attribute_flags(
     libvshadow_store_descriptor_t *store_descriptor,
     uint32_t *attribute_flags,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_attribute_flags";

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	if( attribute_flags == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid attribute flags.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	*attribute_flags = store_descriptor->attribute_flags;

#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );
}

/* Retrieves the number of blocks
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_number_of_blocks(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     int *number_of_blocks,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_number_of_blocks";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	/* This function will acquire the write lock
	 */
	if( libvshadow_store_descriptor_read_block_descriptors(
	     store_descriptor,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read block descriptors.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libcdata_list_get_number_of_elements(
	     store_descriptor->block_descriptors_list,
	     number_of_blocks,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of block descriptors.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves a specific block descriptor
 * Returns 1 if successful or -1 on error
 */
int libvshadow_store_descriptor_get_block_descriptor_by_index(
     libvshadow_store_descriptor_t *store_descriptor,
     libbfio_handle_t *file_io_handle,
     int block_index,
     libvshadow_block_descriptor_t **block_descriptor,
     libcerror_error_t **error )
{
	static char *function = "libvshadow_store_descriptor_get_block_descriptor_by_index";
	int result            = 1;

	if( store_descriptor == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid store descriptor.",
		 function );

		return( -1 );
	}
	/* This function will acquire the write lock
	 */
	if( libvshadow_store_descriptor_read_block_descriptors(
	     store_descriptor,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read block descriptors.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libcdata_list_get_value_by_index(
	     store_descriptor->block_descriptors_list,
	     block_index,
	     (intptr_t **) block_descriptor,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve block descriptor: %d.",
		 function,
		 block_index );

		result = -1;
	}
#if defined( HAVE_LIBVSHADOW_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     store_descriptor->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

