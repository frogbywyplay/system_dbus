/* -*- mode: C; c-file-style: "gnu" -*- */
/* dbus-marshal.c  Marshalling routines
 *
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2003 Red Hat, Inc.
 *
 * Licensed under the Academic Free License version 1.2
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "dbus-marshal.h"
#include "dbus-internals.h"
#define DBUS_CAN_USE_DBUS_STRING_PRIVATE 1
#include "dbus-string-private.h"

#include <string.h>

/**
 * @defgroup DBusMarshal marshaling and unmarshaling
 * @ingroup  DBusInternals
 * @brief functions to marshal/unmarshal data from the wire
 *
 * Types and functions related to converting primitive data types from
 * wire format to native machine format, and vice versa.
 *
 * @{
 */

static dbus_uint32_t
unpack_4_octets (int                  byte_order,
                 const unsigned char *data)
{
  _dbus_assert (_DBUS_ALIGN_ADDRESS (data, 4) == data);
  
  if (byte_order == DBUS_LITTLE_ENDIAN)
    return DBUS_UINT32_FROM_LE (*(dbus_uint32_t*)data);
  else
    return DBUS_UINT32_FROM_BE (*(dbus_uint32_t*)data);
}

#ifndef DBUS_HAVE_INT64
/* from ORBit */
static void
swap_bytes (unsigned char *data,
            unsigned int   len)
{
  unsigned char *p1 = data;
  unsigned char *p2 = data + len - 1;

  while (p1 < p2)
    {
      unsigned char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;

      --p2;
      ++p1;
    }
}
#endif /* !DBUS_HAVE_INT64 */

typedef union
{
#ifdef DBUS_HAVE_INT64
  dbus_int64_t  s;
  dbus_uint64_t u;
#endif
  double d;
} DBusOctets8;

static DBusOctets8
unpack_8_octets (int                  byte_order,
                 const unsigned char *data)
{
  DBusOctets8 r;
  
  _dbus_assert (_DBUS_ALIGN_ADDRESS (data, 8) == data);
  _dbus_assert (sizeof (r) == 8);
  
#ifdef DBUS_HAVE_INT64
  if (byte_order == DBUS_LITTLE_ENDIAN)
    r.u = DBUS_UINT64_FROM_LE (*(dbus_uint64_t*)data);
  else
    r.u = DBUS_UINT64_FROM_BE (*(dbus_uint64_t*)data);
#else
  r.d = *(double*)data;
  swap_bytes (&r, sizeof (r));
#endif
  
  return r;
}

/**
 * Unpacks a 32 bit unsigned integer from a data pointer
 *
 * @param byte_order The byte order to use
 * @param data the data pointer
 * @returns the integer
 */
dbus_uint32_t
_dbus_unpack_uint32 (int                  byte_order,
                     const unsigned char *data)
{
  return unpack_4_octets (byte_order, data);
}  

/**
 * Unpacks a 32 bit signed integer from a data pointer
 *
 * @param byte_order The byte order to use
 * @param data the data pointer
 * @returns the integer
 */
dbus_int32_t
_dbus_unpack_int32 (int                  byte_order,
                    const unsigned char *data)
{
  return (dbus_int32_t) unpack_4_octets (byte_order, data);
}

#ifdef DBUS_HAVE_INT64
/**
 * Unpacks a 64 bit unsigned integer from a data pointer
 *
 * @param byte_order The byte order to use
 * @param data the data pointer
 * @returns the integer
 */
dbus_uint64_t
_dbus_unpack_uint64 (int                  byte_order,
                     const unsigned char *data)
{
  DBusOctets8 r;
  
  r = unpack_8_octets (byte_order, data);

  return r.u;
}  

/**
 * Unpacks a 64 bit signed integer from a data pointer
 *
 * @param byte_order The byte order to use
 * @param data the data pointer
 * @returns the integer
 */
dbus_int64_t
_dbus_unpack_int64 (int                  byte_order,
                    const unsigned char *data)
{
  DBusOctets8 r;
  
  r = unpack_8_octets (byte_order, data);

  return r.s;
}

#endif /* DBUS_HAVE_INT64 */

static void
pack_4_octets (dbus_uint32_t   value,
               int             byte_order,
               unsigned char  *data)
{
  _dbus_assert (_DBUS_ALIGN_ADDRESS (data, 4) == data);
  
  if ((byte_order) == DBUS_LITTLE_ENDIAN)                  
    *((dbus_uint32_t*)(data)) = DBUS_UINT32_TO_LE (value);       
  else
    *((dbus_uint32_t*)(data)) = DBUS_UINT32_TO_BE (value);
}

static void
pack_8_octets (DBusOctets8     value,
               int             byte_order,
               unsigned char  *data)
{
  _dbus_assert (_DBUS_ALIGN_ADDRESS (data, 8) == data);

#ifdef DBUS_HAVE_INT64
  if ((byte_order) == DBUS_LITTLE_ENDIAN)                  
    *((dbus_uint64_t*)(data)) = DBUS_UINT64_TO_LE (value.u); 
  else
    *((dbus_uint64_t*)(data)) = DBUS_UINT64_TO_BE (value.u);
#else
  memcpy (data, &value, 8);
  if (byte_order != DBUS_COMPILER_BYTE_ORDER)
    swap_bytes ((unsigned char *)data, 8);
#endif
}

/**
 * Packs a 32 bit unsigned integer into a data pointer.
 *
 * @param value the value
 * @param byte_order the byte order to use
 * @param data the data pointer
 */
void
_dbus_pack_uint32 (dbus_uint32_t   value,
                   int             byte_order,
                   unsigned char  *data)
{
  pack_4_octets (value, byte_order, data);
}

/**
 * Packs a 32 bit signed integer into a data pointer.
 *
 * @param value the value
 * @param byte_order the byte order to use
 * @param data the data pointer
 */
void
_dbus_pack_int32 (dbus_int32_t   value,
                  int            byte_order,
                  unsigned char *data)
{
  pack_4_octets ((dbus_uint32_t) value, byte_order, data);
}

#ifdef DBUS_HAVE_INT64
/**
 * Packs a 64 bit unsigned integer into a data pointer.
 *
 * @param value the value
 * @param byte_order the byte order to use
 * @param data the data pointer
 */
void
_dbus_pack_uint64 (dbus_uint64_t   value,
                   int             byte_order,
                   unsigned char  *data)
{
  DBusOctets8 r;
  r.u = value;
  pack_8_octets (r, byte_order, data);
}

/**
 * Packs a 64 bit signed integer into a data pointer.
 *
 * @param value the value
 * @param byte_order the byte order to use
 * @param data the data pointer
 */
void
_dbus_pack_int64 (dbus_int64_t   value,
                  int            byte_order,
                  unsigned char *data)
{
  DBusOctets8 r;
  r.s = value;
  pack_8_octets (r, byte_order, data);
}
#endif /* DBUS_HAVE_INT64 */

static void
set_4_octets (DBusString          *str,
              int                  byte_order,
              int                  offset,
              dbus_uint32_t        value)
{
  char *data;
  
  _dbus_assert (byte_order == DBUS_LITTLE_ENDIAN ||
                byte_order == DBUS_BIG_ENDIAN);
  
  data = _dbus_string_get_data_len (str, offset, 4);

  _dbus_pack_uint32 (value, byte_order, data);
}

static void
set_8_octets (DBusString          *str,
              int                  byte_order,
              int                  offset,
              DBusOctets8          value)
{
  char *data;
  
  _dbus_assert (byte_order == DBUS_LITTLE_ENDIAN ||
                byte_order == DBUS_BIG_ENDIAN);
  
  data = _dbus_string_get_data_len (str, offset, 8);

  pack_8_octets (value, byte_order, data);
}

/**
 * Sets the 4 bytes at the given offset to a marshaled signed integer,
 * replacing anything found there previously.
 *
 * @param str the string to write the marshalled int to
 * @param offset the byte offset where int should be written
 * @param byte_order the byte order to use
 * @param value the value
 * 
 */
void
_dbus_marshal_set_int32 (DBusString          *str,
                         int                  byte_order,
                         int                  offset,
                         dbus_int32_t         value)
{
  set_4_octets (str, byte_order, offset, (dbus_uint32_t) value);
}

/**
 * Sets the 4 bytes at the given offset to a marshaled unsigned
 * integer, replacing anything found there previously.
 *
 * @param str the string to write the marshalled int to
 * @param offset the byte offset where int should be written
 * @param byte_order the byte order to use
 * @param value the value
 * 
 */
void
_dbus_marshal_set_uint32 (DBusString          *str,
                          int                  byte_order,
                          int                  offset,
                          dbus_uint32_t        value)
{
  set_4_octets (str, byte_order, offset, value);
}

#ifdef DBUS_HAVE_INT64

/**
 * Sets the 4 bytes at the given offset to a marshaled signed integer,
 * replacing anything found there previously.
 *
 * @param str the string to write the marshalled int to
 * @param offset the byte offset where int should be written
 * @param byte_order the byte order to use
 * @param value the value
 * 
 */
void
_dbus_marshal_set_int64 (DBusString          *str,
                         int                  byte_order,
                         int                  offset,
                         dbus_int64_t         value)
{
  DBusOctets8 r;
  r.s = value;
  set_8_octets (str, byte_order, offset, r);
}

/**
 * Sets the 4 bytes at the given offset to a marshaled unsigned
 * integer, replacing anything found there previously.
 *
 * @param str the string to write the marshalled int to
 * @param offset the byte offset where int should be written
 * @param byte_order the byte order to use
 * @param value the value
 * 
 */
void
_dbus_marshal_set_uint64 (DBusString          *str,
                          int                  byte_order,
                          int                  offset,
                          dbus_uint64_t        value)
{
  DBusOctets8 r;
  r.u = value;
  set_8_octets (str, byte_order, offset, r);
}
#endif /* DBUS_HAVE_INT64 */

/**
 * Sets the existing marshaled string at the given offset with
 * a new marshaled string. The given offset must point to
 * an existing string or the wrong length will be deleted
 * and replaced with the new string.
 *
 * @param str the string to write the marshalled string to
 * @param offset the byte offset where string should be written
 * @param byte_order the byte order to use
 * @param value the value
 * @param len the length to use
 * @returns #TRUE on success
 * 
 */
dbus_bool_t
_dbus_marshal_set_string (DBusString          *str,
                          int                  byte_order,
                          int                  offset,
                          const DBusString    *value,
			  int                  len)
{
  int old_len;
  
  _dbus_assert (byte_order == DBUS_LITTLE_ENDIAN ||
                byte_order == DBUS_BIG_ENDIAN);
  
  old_len = _dbus_demarshal_uint32 (str, byte_order,
                                    offset, NULL);

  if (!_dbus_string_replace_len (value, 0, len,
                                 str, offset + 4, old_len))
    return FALSE;

  _dbus_marshal_set_uint32 (str, byte_order,
                            offset, len);

  return TRUE;
}

static dbus_bool_t
marshal_4_octets (DBusString   *str,
                  int           byte_order,
                  dbus_uint32_t value)
{
  _dbus_assert (sizeof (value) == 4);
  
  if (!_dbus_string_align_length (str, sizeof (dbus_uint32_t)))
    return FALSE;
  
  if (byte_order != DBUS_COMPILER_BYTE_ORDER)
    value = DBUS_UINT32_SWAP_LE_BE (value);

  return _dbus_string_append_len (str, (const char *)&value, sizeof (dbus_uint32_t));
}

static dbus_bool_t
marshal_8_octets (DBusString *str,
                  int         byte_order,
                  DBusOctets8 value)
{
  _dbus_assert (sizeof (value) == 8);

  if (!_dbus_string_align_length (str, 8))
    return FALSE;
  
  if (byte_order != DBUS_COMPILER_BYTE_ORDER)
    pack_8_octets (value, byte_order, (unsigned char*) &value); /* pack into self, swapping as we go */

  return _dbus_string_append_len (str, (const char *)&value, 8);
}

/**
 * Marshals a double value.
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the value
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_double (DBusString *str,
		      int         byte_order,
		      double      value)
{
  DBusOctets8 r;
  r.d = value;
  return marshal_8_octets (str, byte_order, r);
}

/**
 * Marshals a 32 bit signed integer value.
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the value
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_int32  (DBusString   *str,
		      int           byte_order,
		      dbus_int32_t  value)
{
  return marshal_4_octets (str, byte_order, (dbus_uint32_t) value);
}

/**
 * Marshals a 32 bit unsigned integer value.
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the value
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_uint32 (DBusString    *str,
		      int            byte_order,
		      dbus_uint32_t  value)
{
  return marshal_4_octets (str, byte_order, value);
}


#ifdef DBUS_HAVE_INT64
/**
 * Marshals a 64 bit signed integer value.
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the value
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_int64  (DBusString   *str,
		      int           byte_order,
		      dbus_int64_t  value)
{
  DBusOctets8 r;
  r.s = value;
  return marshal_8_octets (str, byte_order, r);
}

/**
 * Marshals a 64 bit unsigned integer value.
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the value
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_uint64 (DBusString    *str,
		      int            byte_order,
		      dbus_uint64_t  value)
{
  DBusOctets8 r;
  r.u = value;
  return marshal_8_octets (str, byte_order, r);
}

#endif /* DBUS_HAVE_INT64 */

/**
 * Marshals a UTF-8 string
 *
 * @todo: If the string append fails we need to restore
 * the old length. (also for other marshallers)
 * 
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the string
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_string (DBusString    *str,
		      int            byte_order,
		      const char    *value)
{
  int len, old_string_len;

  old_string_len = _dbus_string_get_length (str);
  
  len = strlen (value);

  if (!_dbus_marshal_uint32 (str, byte_order, len))
    {
      /* Restore the previous length */
      _dbus_string_set_length (str, old_string_len);

      return FALSE;
    }

  return _dbus_string_append_len (str, value, len + 1);
}

/**
 * Marshals a byte array
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the array
 * @param len number of elements in the array
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_byte_array (DBusString          *str,
			  int                  byte_order,
			  const unsigned char *value,
			  int                  len)
{
  int old_string_len;

  old_string_len = _dbus_string_get_length (str);
  
  if (!_dbus_marshal_uint32 (str, byte_order, len))
    {
      /* Restore the previous length */
      _dbus_string_set_length (str, old_string_len);

      return FALSE;
    }

  if (len == 0)
    return TRUE;
  else
    return _dbus_string_append_len (str, value, len);
}

static dbus_bool_t
marshal_4_octets_array (DBusString          *str,
                        int                  byte_order,
                        const dbus_uint32_t *value,
                        int                  len)
{
  int old_string_len;
  int array_start;

  old_string_len = _dbus_string_get_length (str);

  if (!_dbus_marshal_uint32 (str, byte_order, len * 4))
    goto error;

  array_start = _dbus_string_get_length (str);
  
  if (!_dbus_string_append_len (str, (const unsigned char*) value,
                                len * 4))
    goto error;
  
  if (byte_order != DBUS_COMPILER_BYTE_ORDER)
    {
      const unsigned char *d;
      const unsigned char *end;
      
      d = _dbus_string_get_data (str) + array_start;
      end = d + len * 4;
      while (d != end)
        {
          *((dbus_uint32_t*)d) = DBUS_UINT32_SWAP_LE_BE (*((dbus_uint32_t*)d));
          d += 4;
        }
    }

  return TRUE;
  
 error:
  /* Restore previous length */
  _dbus_string_set_length (str, old_string_len);
  
  return FALSE;  
}

static dbus_bool_t
marshal_8_octets_array (DBusString          *str,
                        int                  byte_order,
                        const DBusOctets8   *value,
                        int                  len)
{
  int old_string_len;
  int array_start;

  old_string_len = _dbus_string_get_length (str);

  if (!_dbus_marshal_uint32 (str, byte_order, len * 8))
    goto error;

  array_start = _dbus_string_get_length (str);
  
  if (!_dbus_string_append_len (str, (const unsigned char*) value,
                                len * 8))
    goto error;
  
  if (byte_order != DBUS_COMPILER_BYTE_ORDER)
    {
      const unsigned char *d;
      const unsigned char *end;
      
      d = _dbus_string_get_data (str) + array_start;
      end = d + len * 8;
      while (d != end)
        {
#ifdef DBUS_HAVE_INT64
          *((dbus_uint64_t*)d) = DBUS_UINT64_SWAP_LE_BE (*((dbus_uint64_t*)d));
#else
          swap_bytes (d, 8);
#endif
          d += 8;
        }
    }

  return TRUE;
  
 error:
  /* Restore previous length */
  _dbus_string_set_length (str, old_string_len);
  
  return FALSE;  
}

/**
 * Marshals a 32 bit signed integer array
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the array
 * @param len the length of the array
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_int32_array (DBusString         *str,
			   int                 byte_order,
			   const dbus_int32_t *value,
			   int                 len)
{
  return marshal_4_octets_array (str, byte_order,
                                 (const dbus_uint32_t*) value,
                                 len);
}

/**
 * Marshals a 32 bit unsigned integer array
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the array
 * @param len the length of the array
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_uint32_array (DBusString          *str,
			    int                  byte_order,
			    const dbus_uint32_t  *value,
			    int                  len)
{
  return marshal_4_octets_array (str, byte_order,
                                 value,
                                 len);
}

#ifdef DBUS_HAVE_INT64

/**
 * Marshals a 64 bit signed integer array
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the array
 * @param len the length of the array
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_int64_array (DBusString         *str,
			   int                 byte_order,
			   const dbus_int64_t *value,
			   int                 len)
{
  return marshal_8_octets_array (str, byte_order,
                                 (const DBusOctets8*) value,
                                 len);
}

/**
 * Marshals a 64 bit unsigned integer array
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the array
 * @param len the length of the array
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_uint64_array (DBusString          *str,
			    int                  byte_order,
			    const dbus_uint64_t  *value,
			    int                  len)
{
  return marshal_8_octets_array (str, byte_order,
                                 (const DBusOctets8*) value,
                                 len);
}

#endif /* DBUS_HAVE_INT64 */

/**
 * Marshals a double array
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the array
 * @param len the length of the array
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_double_array (DBusString          *str,
			    int                  byte_order,
			    const double        *value,
			    int                  len)
{
  return marshal_8_octets_array (str, byte_order,
                                 (const DBusOctets8*) value,
                                 len);
}

/**
 * Marshals a string array
 *
 * @param str the string to append the marshalled value to
 * @param byte_order the byte order to use
 * @param value the array
 * @param len the length of the array
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_marshal_string_array (DBusString  *str,
			    int          byte_order,
			    const char **value,
			    int          len)
{
  int i, old_string_len, array_start;

  old_string_len = _dbus_string_get_length (str);

  /* Set the length to 0 temporarily */
  if (!_dbus_marshal_uint32 (str, byte_order, 0))
    goto error;

  array_start = _dbus_string_get_length (str);
  
  for (i = 0; i < len; i++)
    if (!_dbus_marshal_string (str, byte_order, value[i]))
      goto error;

  /* Write the length now that we know it */
  _dbus_marshal_set_uint32 (str, byte_order,
			    _DBUS_ALIGN_VALUE (old_string_len, sizeof(dbus_uint32_t)),
			    _dbus_string_get_length (str) - array_start);
  
  return TRUE;
  
 error:
  /* Restore previous length */
  _dbus_string_set_length (str, old_string_len);
  
  return FALSE;      
}

static dbus_uint32_t
demarshal_4_octets (const DBusString *str,
                    int               byte_order,
                    int               pos,
                    int              *new_pos)
{
  const DBusRealString *real = (const DBusRealString*) str;
  
  pos = _DBUS_ALIGN_VALUE (pos, 4);
  
  if (new_pos)
    *new_pos = pos + 4;

  return unpack_4_octets (byte_order, real->str + pos);
}

static DBusOctets8
demarshal_8_octets (const DBusString *str,
                    int               byte_order,
                    int               pos,
                    int              *new_pos)
{
  const DBusRealString *real = (const DBusRealString*) str;
  
  pos = _DBUS_ALIGN_VALUE (pos, 8);
  
  if (new_pos)
    *new_pos = pos + 8;

  return unpack_8_octets (byte_order, real->str + pos);
}

/**
 * Demarshals a double.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @returns the demarshaled double.
 */
double
_dbus_demarshal_double (const DBusString  *str,
			int                byte_order,
			int                pos,
			int               *new_pos)
{
  DBusOctets8 r;

  r = demarshal_8_octets (str, byte_order, pos, new_pos);

  return r.d;
}

/**
 * Demarshals a 32 bit signed integer.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @returns the demarshaled integer.
 */
dbus_int32_t
_dbus_demarshal_int32  (const DBusString *str,
			int               byte_order,
			int               pos,
			int              *new_pos)
{
  return (dbus_int32_t) demarshal_4_octets (str, byte_order, pos, new_pos);
}

/**
 * Demarshals a 32 bit unsigned integer.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @returns the demarshaled integer.
 */
dbus_uint32_t
_dbus_demarshal_uint32  (const DBusString *str,
			 int         byte_order,
			 int         pos,
			 int        *new_pos)
{
  return demarshal_4_octets (str, byte_order, pos, new_pos);
}

#ifdef DBUS_HAVE_INT64

/**
 * Demarshals a 64 bit signed integer.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @returns the demarshaled integer.
 */
dbus_int64_t
_dbus_demarshal_int64  (const DBusString *str,
			int               byte_order,
			int               pos,
			int              *new_pos)
{
  DBusOctets8 r;

  r = demarshal_8_octets (str, byte_order, pos, new_pos);

  return r.s;
}

/**
 * Demarshals a 64 bit unsigned integer.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @returns the demarshaled integer.
 */
dbus_uint64_t
_dbus_demarshal_uint64  (const DBusString *str,
			 int         byte_order,
			 int         pos,
			 int        *new_pos)
{
  DBusOctets8 r;

  r = demarshal_8_octets (str, byte_order, pos, new_pos);

  return r.u;
}

#endif /* DBUS_HAVE_INT64 */

/**
 * Demarshals an UTF-8 string.
 *
 * @todo Should we check the string to make sure
 * that it's  valid UTF-8, and maybe "fix" the string
 * if it's broken?
 *
 * @todo Should probably demarshal to a DBusString,
 * having memcpy() in here is Evil(tm).
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @returns the demarshaled string.
 */
char *
_dbus_demarshal_string (const DBusString *str,
			int               byte_order,
			int               pos,
			int              *new_pos)
{
  int len;
  char *retval;
  const char *data;
  
  len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);

  retval = dbus_malloc (len + 1);

  if (!retval)
    return NULL;

  data = _dbus_string_get_const_data_len (str, pos, len + 1);

  if (!data)
    return NULL;

  memcpy (retval, data, len + 1);

  if (new_pos)
    *new_pos = pos + len + 1;
  
  return retval;
}

/**
 * Demarshals a byte array.
 *
 * @todo Should probably demarshal to a DBusString,
 * having memcpy() in here is Evil(tm).
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @param array the array
 * @param array_len length of the demarshaled data
 
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_demarshal_byte_array (const DBusString  *str,
			    int                byte_order,
			    int                pos,
			    int               *new_pos,
			    unsigned char    **array,
			    int               *array_len)
{
  int len;
  unsigned char *retval;
  const char *data;

  len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);

  if (len == 0)
    {
      *array_len = len;
      *array = NULL;

      if (new_pos)
	*new_pos = pos;
      
      return TRUE;
    }
  
  retval = dbus_malloc (len);

  if (!retval)
    return FALSE;

  data = _dbus_string_get_const_data_len (str, pos, len);

  if (!data)
    {
      dbus_free (retval);
      return FALSE;
    }

  memcpy (retval, data, len);

  if (new_pos)
    *new_pos = pos + len;

  *array = retval;
  *array_len = len;
  
  return TRUE;
}

static dbus_bool_t
demarshal_4_octets_array (const DBusString  *str,
                          int                byte_order,
                          int                pos,
                          int               *new_pos,
                          dbus_uint32_t    **array,
                          int               *array_len)
{
  int len, i;
  dbus_uint32_t *retval;
  int byte_len;
  
  byte_len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);
  len = byte_len / 4;

  if (len == 0)
    {
      *array_len = 0;
      *array = NULL;

      if (new_pos)
	*new_pos = pos;
      
      return TRUE;
    }

  if (!_dbus_string_copy_data_len (str, (char**) &retval,
                                   pos, byte_len))
    return FALSE;
  
  if (byte_order != DBUS_COMPILER_BYTE_ORDER)
    {
      for (i = 0; i < len; i++)
        retval[i] = DBUS_UINT32_SWAP_LE_BE (retval[i]);
    }

  if (new_pos)
    *new_pos = pos + byte_len;

  *array_len = len;
  *array = retval;
  
  return TRUE;  
}

static dbus_bool_t
demarshal_8_octets_array (const DBusString  *str,
                          int                byte_order,
                          int                pos,
                          int               *new_pos,
                          DBusOctets8      **array,
                          int               *array_len)
{
  int len, i;
  DBusOctets8 *retval;
  int byte_len;
  
  byte_len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);
  len = byte_len / 8;

  if (len == 0)
    {
      *array_len = 0;
      *array = NULL;

      if (new_pos)
	*new_pos = pos;
      
      return TRUE;
    }

  if (!_dbus_string_copy_data_len (str, (char**) &retval,
                                   pos, byte_len))
    return FALSE;
  
  if (byte_order != DBUS_COMPILER_BYTE_ORDER)
    {
      for (i = 0; i < len; i++)
        {
#ifdef DBUS_HAVE_INT64
          retval[i].u = DBUS_UINT64_SWAP_LE_BE (retval[i].u);
#else
          swap_bytes (&retval[i], 8);
#endif
        }
    }

  if (new_pos)
    *new_pos = pos + byte_len;

  *array_len = len;
  *array = retval;
  
  return TRUE;  
}

/**
 * Demarshals a 32 bit signed integer array.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @param array the array
 * @param array_len length of the demarshaled data
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_demarshal_int32_array (const DBusString  *str,
			     int                byte_order,
			     int                pos,
			     int               *new_pos,
			     dbus_int32_t     **array,
			     int               *array_len)
{
  return demarshal_4_octets_array (str, byte_order, pos, new_pos,
                                   (dbus_uint32_t**) array, array_len);
}

/**
 * Demarshals a 32 bit unsigned integer array.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @param array the array
 * @param array_len length of the demarshaled data
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_demarshal_uint32_array (const DBusString  *str,
			      int                byte_order,
			      int                pos,
			      int               *new_pos,
			      dbus_uint32_t    **array,
			      int               *array_len)
{
  return demarshal_4_octets_array (str, byte_order, pos, new_pos,
                                   array, array_len);
}

#ifdef DBUS_HAVE_INT64

/**
 * Demarshals a 64 bit signed integer array.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @param array the array
 * @param array_len length of the demarshaled data
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_demarshal_int64_array (const DBusString  *str,
			     int                byte_order,
			     int                pos,
			     int               *new_pos,
			     dbus_int64_t     **array,
			     int               *array_len)
{
  return demarshal_8_octets_array (str, byte_order, pos, new_pos,
                                   (DBusOctets8**) array, array_len);
}

/**
 * Demarshals a 64 bit unsigned integer array.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @param array the array
 * @param array_len length of the demarshaled data
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_demarshal_uint64_array (const DBusString  *str,
			      int                byte_order,
			      int                pos,
			      int               *new_pos,
			      dbus_uint64_t    **array,
			      int               *array_len)
{
  return demarshal_8_octets_array (str, byte_order, pos, new_pos,
                                   (DBusOctets8**) array, array_len);
}

#endif /* DBUS_HAVE_INT64 */

/**
 * Demarshals a double array.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @param array the array
 * @param array_len length of the demarshaled data
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_demarshal_double_array (const DBusString  *str,
			      int                byte_order,
			      int                pos,
			      int               *new_pos,
			      double           **array,
			      int               *array_len)
{
  return demarshal_8_octets_array (str, byte_order, pos, new_pos,
                                   (DBusOctets8**) array, array_len);
}

/**
 * Demarshals a string array.
 *
 * @param str the string containing the data
 * @param byte_order the byte order
 * @param pos the position in the string
 * @param new_pos the new position of the string
 * @param array the array
 * @param array_len length of the demarshaled data
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_demarshal_string_array (const DBusString   *str,
			      int                 byte_order,
			      int                 pos,
			      int                *new_pos,
			      char             ***array,
			      int                *array_len)
{
  int bytes_len, i;
  int len, allocated;
  int end_pos;
  char **retval;
  
  bytes_len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);
  
  if (bytes_len == 0)
    {
      *array_len = 0;
      *array = NULL;

      if (new_pos)
	*new_pos = pos;
      
      return TRUE;
    }

  len = 0;
  allocated = 4;
  end_pos = pos + bytes_len;
  
  retval = dbus_new (char *, allocated);

  if (!retval)
    return FALSE;

  while (pos < end_pos)
    {
      retval[len] = _dbus_demarshal_string (str, byte_order, pos, &pos);
      
      if (retval[len] == NULL)
	goto error;
      
      len += 1;

      if (len >= allocated - 1) /* -1 for NULL termination */
        {
          char **newp;
          newp = dbus_realloc (retval,
                               sizeof (char*) * allocated * 2);
          if (newp == NULL)
            goto error;

          allocated *= 2;
          retval = newp;
        }
    }
      
  retval[len] = NULL;

  if (new_pos)
    *new_pos = pos;
  
  *array = retval;
  *array_len = len;
  
  return TRUE;

 error:
  for (i = 0; i < len; i++)
    dbus_free (retval[i]);
  dbus_free (retval);

  return FALSE;
}

/** 
 * Returns the position right after the end of an argument.  PERFORMS
 * NO VALIDATION WHATSOEVER. The message must have been previously
 * validated.
 *
 * @param str a string
 * @param byte_order the byte order to use
 * @param type the type of the argument
 * @param pos the pos where the arg starts
 * @param end_pos pointer where the position right
 * after the end position will follow
 * @returns TRUE if more data exists after the arg
 */
dbus_bool_t
_dbus_marshal_get_arg_end_pos (const DBusString *str,
                               int               byte_order,
			       int               type,
                               int               pos,
                               int              *end_pos)
{
  if (pos >= _dbus_string_get_length (str))
    return FALSE;

  switch (type)
    {
    case DBUS_TYPE_INVALID:
      return FALSE;
      break;

    case DBUS_TYPE_NIL:
      *end_pos = pos;
      break;

    case DBUS_TYPE_BYTE:
      *end_pos = pos + 1;
      break;
      
    case DBUS_TYPE_BOOLEAN:
      *end_pos = pos + 1;
      break;

    case DBUS_TYPE_INT32:
      *end_pos = _DBUS_ALIGN_VALUE (pos, sizeof (dbus_int32_t)) + sizeof (dbus_int32_t);

      break;

    case DBUS_TYPE_UINT32:
      *end_pos = _DBUS_ALIGN_VALUE (pos, sizeof (dbus_uint32_t)) + sizeof (dbus_uint32_t);

      break;

#ifdef DBUS_HAVE_INT64
    case DBUS_TYPE_INT64:
      *end_pos = _DBUS_ALIGN_VALUE (pos, sizeof (dbus_int64_t)) + sizeof (dbus_int64_t);

      break;

    case DBUS_TYPE_UINT64:
      *end_pos = _DBUS_ALIGN_VALUE (pos, sizeof (dbus_uint64_t)) + sizeof (dbus_uint64_t);

      break;
#endif /* DBUS_HAVE_INT64 */
      
    case DBUS_TYPE_DOUBLE:
      *end_pos = _DBUS_ALIGN_VALUE (pos, sizeof (double)) + sizeof (double);

      break;

    case DBUS_TYPE_STRING:
      {
	int len;
	
	/* Demarshal the length */
	len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);

	*end_pos = pos + len + 1;
      }
      break;

    case DBUS_TYPE_NAMED:
      {
	int len;
	
	/* Demarshal the string length */
	len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);

	*end_pos = pos + len + 1;
	
	/* Demarshal the data length */
	len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);

	*end_pos = pos + len;
      }
      break;
      
    case DBUS_TYPE_ARRAY:
      {
	int len;

	/* Demarshal the length  */
	len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);
	
	*end_pos = pos + len;
      }
      break;

    case DBUS_TYPE_DICT:
      {
	int len;

	/* Demarshal the length */
	len = _dbus_demarshal_uint32 (str, byte_order, pos, &pos);
	
	*end_pos = pos + len;
      }
      break;
      
    default:
      _dbus_warn ("Unknown message arg type %d\n", type);
      _dbus_assert_not_reached ("Unknown message argument type\n");
      return FALSE;
    }

  if (*end_pos > _dbus_string_get_length (str))
    return FALSE;
  
  return TRUE;
}

/**
 * Demarshals and validates a length; returns < 0 if the validation
 * fails. The length is required to be small enough that
 * len*sizeof(double) will not overflow, and small enough to fit in a
 * signed integer. DOES NOT check whether the length points
 * beyond the end of the string, because it doesn't know the
 * size of array elements.
 *
 * @param str the string
 * @param byte_order the byte order
 * @param pos the unaligned string position (snap to next aligned)
 * @param new_pos return location for new position.
 */
static int
demarshal_and_validate_len (const DBusString *str,
                            int               byte_order,
                            int               pos,
                            int              *new_pos)
{
  int align_4 = _DBUS_ALIGN_VALUE (pos, 4);
  unsigned int len;

  _dbus_assert (new_pos != NULL);
  
  if ((align_4 + 4) > _dbus_string_get_length (str))
    {
      _dbus_verbose ("not enough room in message for array length\n");
      return -1;
    }
  
  if (!_dbus_string_validate_nul (str, pos,
                                  align_4 - pos))
    {
      _dbus_verbose ("array length alignment padding not initialized to nul\n");
      return -1;
    }

  len = _dbus_demarshal_uint32 (str, byte_order, align_4, new_pos);

  /* note that the len is the number of bytes, so we need it to be
   * at least SIZE_T_MAX, but make it smaller just to keep things
   * sane.  We end up using ints for most sizes to avoid unsigned mess
   * so limit to maximum 32-bit signed int divided by at least 8, more
   * for a bit of paranoia margin. INT_MAX/32 is about 65 megabytes.
   */  
#define MAX_ARRAY_LENGTH (((unsigned int)_DBUS_INT_MAX) / 32)
  if (len > MAX_ARRAY_LENGTH)
    {
      _dbus_verbose ("array length %u exceeds maximum of %u\n",
                     len, MAX_ARRAY_LENGTH);
      return -1;
    }
  else
    return (int) len;
}

static dbus_bool_t
validate_string (const DBusString *str,
                 int               pos,
                 int               len_without_nul,
                 int              *end_pos)
{
  *end_pos = pos + len_without_nul + 1;
  
  if (*end_pos > _dbus_string_get_length (str))
    {
      _dbus_verbose ("string length outside length of the message\n");
      return FALSE;
    }
  
  if (_dbus_string_get_byte (str, pos + len_without_nul) != '\0')
    {
      _dbus_verbose ("string arg not nul-terminated\n");
      return FALSE;
    }
  
  if (!_dbus_string_validate_utf8 (str, pos, len_without_nul))
    {
      _dbus_verbose ("string is not valid UTF-8\n");
      return FALSE;
    }

  return TRUE;
}   

/**
 * Validates and returns a typecode at a specific position
 * in the message
 *
 * @param str a string
 * @param type the type of the argument
 * @param pos the pos where the typecode starts
 * @param end_pos pointer where the position right
 * after the end position will follow
 * @returns #TRUE if the type is valid.
 */
dbus_bool_t
_dbus_marshal_validate_type   (const DBusString *str,
			       int               pos,
			       int              *type,
			       int              *end_pos)
{
  const char *data;
  
  if (pos >= _dbus_string_get_length (str))
    return FALSE;

  data = _dbus_string_get_const_data_len (str, pos, 1);

  if (*data > DBUS_TYPE_INVALID && *data <= DBUS_TYPE_LAST)
    {
      *type = *data;
      if (end_pos != NULL)
	*end_pos = pos + 1;
      return TRUE;
    }
  
  return FALSE;
}


/** 
 * Validates an argument of a specific type, checking that it
 * is well-formed, for example no ludicrous length fields, strings
 * are nul-terminated, etc.
 * Returns the end position of the argument in end_pos, and
 * returns #TRUE if a valid arg begins at "pos"
 *
 * @todo security: need to audit this function.
 *
 * @todo For array types that can't be invalid, we should not
 * walk the whole array validating it. e.g. just skip all the
 * int values in an int array.
 * 
 * @param str a string
 * @param byte_order the byte order to use
 * @param depth current recursion depth, to prevent excessive recursion
 * @param type the type of the argument
 * @param array_type_pos the position of the current array type, or
 *        -1 if not in an array
 * @param pos the pos where the arg starts
 * @param end_pos pointer where the position right
 * after the end position will follow
 * @returns #TRUE if the arg is valid.
 */
dbus_bool_t
_dbus_marshal_validate_arg (const DBusString *str,
                            int	              byte_order,
                            int               depth,
			    int               type,
			    int               array_type_pos,
                            int               pos,
                            int              *end_pos)
{
  if (pos > _dbus_string_get_length (str))
    {
      _dbus_verbose ("Validation went off the end of the message\n");
      return FALSE;
    }

#define MAX_VALIDATION_DEPTH 32
  
  if (depth > MAX_VALIDATION_DEPTH)
    {
      _dbus_verbose ("Maximum recursion depth reached validating message\n");
      return FALSE;
    }
  
  switch (type)
    {
    case DBUS_TYPE_INVALID:
      return FALSE;
      break;

    case DBUS_TYPE_NIL:
      *end_pos = pos;
      break;

    case DBUS_TYPE_BYTE:
      if (1 > _dbus_string_get_length (str) - pos)
	{
	  _dbus_verbose ("no room for byte value\n");
	  return FALSE;
	}
	
      *end_pos = pos + 1;
      break;
      
    case DBUS_TYPE_BOOLEAN:
      {
	unsigned char c;

        if (1 > _dbus_string_get_length (str) - pos)
          {
            _dbus_verbose ("no room for boolean value\n");
            return FALSE;
          }
        
	c = _dbus_string_get_byte (str, pos);

	if (c != 0 && c != 1)
	  {
	    _dbus_verbose ("boolean value must be either 0 or 1, not %d\n", c);
	    return FALSE;
	  }
	
        *end_pos = pos + 1;
      }
      break;
      
    case DBUS_TYPE_INT32:
    case DBUS_TYPE_UINT32:
      {
        int align_4 = _DBUS_ALIGN_VALUE (pos, 4);
        
        if (!_dbus_string_validate_nul (str, pos,
                                        align_4 - pos))
          {
            _dbus_verbose ("int32/uint32 alignment padding not initialized to nul\n");
            return FALSE;
          }

        *end_pos = align_4 + 4;
      }
      break;

    case DBUS_TYPE_INT64:
    case DBUS_TYPE_UINT64:
      {
        int align_8 = _DBUS_ALIGN_VALUE (pos, 8);
        
        if (!_dbus_string_validate_nul (str, pos,
                                        align_8 - pos))
          {
            _dbus_verbose ("int64/uint64 alignment padding not initialized to nul\n");
            return FALSE;
          }

        *end_pos = align_8 + 8;
      }
      break;
      
    case DBUS_TYPE_DOUBLE:
      {
        int align_8 = _DBUS_ALIGN_VALUE (pos, 8);

        _dbus_verbose_bytes_of_string (str, pos, (align_8 + 8 - pos));
        
        if (!_dbus_string_validate_nul (str, pos,
                                        align_8 - pos))
          {
            _dbus_verbose ("double alignment padding not initialized to nul\n");
            return FALSE;
          }

        *end_pos = align_8 + 8;
      }
      break;

    case DBUS_TYPE_STRING:
      {
	int len;

	/* Demarshal the length, which does NOT include
         * nul termination
         */
	len = demarshal_and_validate_len (str, byte_order, pos, &pos);
        if (len < 0)
          return FALSE;

        if (!validate_string (str, pos, len, end_pos))
          return FALSE;
      }
      break;

    case DBUS_TYPE_NAMED:
      {
	int len;

	/* Demarshal the string length, which does NOT include
         * nul termination
         */
	len = demarshal_and_validate_len (str, byte_order, pos, &pos);
        if (len < 0)
          return FALSE;

        if (!validate_string (str, pos, len, &pos))
          return FALSE;

	/* Validate data */
	len = demarshal_and_validate_len (str, byte_order, pos, &pos);
        if (len < 0)
          return FALSE;

	*end_pos = pos + len;
      }
      break;
      
    case DBUS_TYPE_ARRAY:
      {
	int len;
	int end;
	int array_type;

	if (array_type_pos == -1)
	  {
	    array_type_pos = pos;

	    do
	      {
		if (!_dbus_marshal_validate_type (str, pos, &array_type, &pos))
		  {
		    _dbus_verbose ("invalid array type\n");
		    return FALSE;
		  }
		
		/* NIL values take up no space, so you couldn't iterate over an array of them.
		 * array of nil seems useless anyway; the useful thing might be array of
		 * (nil OR string) but we have no framework for that.
		 */
		if (array_type == DBUS_TYPE_NIL)
		  {
		    _dbus_verbose ("array of NIL is not allowed\n");
		    return FALSE;
		  }
	      }
	    while (array_type == DBUS_TYPE_ARRAY);
	  }
	else
	  array_type_pos++;

	if (!_dbus_marshal_validate_type (str, array_type_pos, &array_type, NULL))
	  {
	    _dbus_verbose ("invalid array type\n");
	    return FALSE;
	  }
        
	len = demarshal_and_validate_len (str, byte_order, pos, &pos);
        if (len < 0)
          return FALSE;

        if (len > _dbus_string_get_length (str) - pos)
          {
            _dbus_verbose ("array length outside length of the message\n");
            return FALSE;
          }
	
	end = pos + len;
        
	while (pos < end)
	  {
	    if (!_dbus_marshal_validate_arg (str, byte_order, depth + 1,
					     array_type, array_type_pos, pos, &pos))
	      return FALSE;
	  }

        if (pos < end)
          {
            /* This should not be able to happen, as long as validate_arg moves forward;
             * but the check is here just to be paranoid.
             */
            _dbus_verbose ("array length %d specified was longer than actual array contents by %d\n",
                    len, end - pos);
            return FALSE;
          }
        
	if (pos > end)
	  {
	    _dbus_verbose ("array contents exceeds array length %d by %d\n", len, pos - end);
	    return FALSE;
	  }

	*end_pos = pos;
      }
      break;

    case DBUS_TYPE_DICT:
      {
	int dict_type;
	int len;
	int end;
	
	len = demarshal_and_validate_len (str, byte_order, pos, &pos);
        if (len < 0)
          return FALSE;

        if (len > _dbus_string_get_length (str) - pos)
          {
            _dbus_verbose ("dict length outside length of the message\n");
            return FALSE;
          }
	
	end = pos + len;
	
	while (pos < end)
	  {
	    /* Validate name */
	    if (!_dbus_marshal_validate_arg (str, byte_order, depth + 1,
					     DBUS_TYPE_STRING, -1, pos, &pos))
	      return FALSE;
	    
	    if (!_dbus_marshal_validate_type (str, pos, &dict_type, &pos))
	      {
		_dbus_verbose ("invalid dict entry type at offset %d\n", pos);
		return FALSE;
	      }
	    
	    /* Validate element */
	    if (!_dbus_marshal_validate_arg (str, byte_order, depth + 1,
					     dict_type, -1, pos, &pos))
	      return FALSE;
	  }
	
	if (pos > end)
	  {
	    _dbus_verbose ("dict contents exceed stated dict length\n");
	    return FALSE;
	  }
        
	*end_pos = pos;
      }
      break;
      
    default:
      _dbus_verbose ("Unknown message arg type %d\n", type);
      return FALSE;
    }

  if (*end_pos > _dbus_string_get_length (str))
    return FALSE;
  
  return TRUE;
}


/**
 * If in verbose mode, print a block of binary data.
 *
 * @todo right now it prints even if not in verbose mode
 * 
 * @param data the data
 * @param len the length of the data
 */
void
_dbus_verbose_bytes (const unsigned char *data,
                     int                  len)
{
  int i;
  const unsigned char *aligned;

  _dbus_assert (len >= 0);
  
  /* Print blanks on first row if appropriate */
  aligned = _DBUS_ALIGN_ADDRESS (data, 4);
  if (aligned > data)
    aligned -= 4;
  _dbus_assert (aligned <= data);

  if (aligned != data)
    {
      _dbus_verbose ("%4d\t%p: ", - (data - aligned), aligned); 
      while (aligned != data)
        {
          _dbus_verbose ("    ");
          ++aligned;
        }
    }

  /* now print the bytes */
  i = 0;
  while (i < len)
    {
      if (_DBUS_ALIGN_ADDRESS (&data[i], 4) == &data[i])
        {
          _dbus_verbose ("%4d\t%p: ",
                   i, &data[i]);
        }
      
      if (data[i] >= 32 &&
          data[i] <= 126)
        _dbus_verbose (" '%c' ", data[i]);
      else
        _dbus_verbose ("0x%s%x ",
                 data[i] <= 0xf ? "0" : "", data[i]);

      ++i;

      if (_DBUS_ALIGN_ADDRESS (&data[i], 4) == &data[i])
        {
          if (i > 3)
            _dbus_verbose ("BE: %d LE: %d",
                           _dbus_unpack_uint32 (DBUS_BIG_ENDIAN, &data[i-4]),
                           _dbus_unpack_uint32 (DBUS_LITTLE_ENDIAN, &data[i-4]));

          if (i > 7 && 
              _DBUS_ALIGN_ADDRESS (&data[i], 8) == &data[i])
            {
              _dbus_verbose (" dbl: %g",
                             *(double*)&data[i-8]);
            }
          
          _dbus_verbose ("\n");
        }
    }

  _dbus_verbose ("\n");
}

/**
 * Dump the given part of the string to verbose log.
 *
 * @param str the string
 * @param start the start of range to dump
 * @param len length of range
 */
void
_dbus_verbose_bytes_of_string (const DBusString    *str,
                               int                  start,
                               int                  len)
{
  const char *d;
  int real_len;

  real_len = _dbus_string_get_length (str);

  _dbus_assert (start >= 0);
  
  if (start > real_len)
    {
      _dbus_verbose ("  [%d,%d) is not inside string of length %d\n",
                     start, len, real_len);
      return;
    }

  if ((start + len) > real_len)
    {
      _dbus_verbose ("  [%d,%d) extends outside string of length %d\n",
                     start, len, real_len);
      len = real_len - start;
    }
  
  d = _dbus_string_get_const_data_len (str, start, len);

  _dbus_verbose_bytes (d, len);
}

/** @} */

#ifdef DBUS_BUILD_TESTS
#include "dbus-test.h"
#include <stdio.h>

dbus_bool_t
_dbus_marshal_test (void)
{
  DBusString str;
  char *tmp1, *tmp2;
  int pos = 0, len;
  dbus_int32_t array1[3] = { 0x123, 0x456, 0x789 }, *array2;
#ifdef DBUS_HAVE_INT64
  dbus_int64_t array3[3] = { 0x123ffffffff, 0x456ffffffff, 0x789ffffffff }, *array4;
#endif
  
  if (!_dbus_string_init (&str))
    _dbus_assert_not_reached ("failed to init string");

  /* Marshal doubles */
  if (!_dbus_marshal_double (&str, DBUS_BIG_ENDIAN, 3.14))
    _dbus_assert_not_reached ("could not marshal double value");
  if (!_dbus_demarshal_double (&str, DBUS_BIG_ENDIAN, pos, &pos) == 3.14)
    _dbus_assert_not_reached ("demarshal failed");

  if (!_dbus_marshal_double (&str, DBUS_LITTLE_ENDIAN, 3.14))
    _dbus_assert_not_reached ("could not marshal double value");
  if (!_dbus_demarshal_double (&str, DBUS_LITTLE_ENDIAN, pos, &pos) == 3.14)
    _dbus_assert_not_reached ("demarshal failed");
  
  /* Marshal signed integers */
  if (!_dbus_marshal_int32 (&str, DBUS_BIG_ENDIAN, -12345678))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_int32 (&str, DBUS_BIG_ENDIAN, pos, &pos) == -12345678)
    _dbus_assert_not_reached ("demarshal failed");

  if (!_dbus_marshal_int32 (&str, DBUS_LITTLE_ENDIAN, -12345678))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_int32 (&str, DBUS_LITTLE_ENDIAN, pos, &pos) == -12345678)
    _dbus_assert_not_reached ("demarshal failed");
  
  /* Marshal unsigned integers */
  if (!_dbus_marshal_uint32 (&str, DBUS_BIG_ENDIAN, 0x12345678))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_uint32 (&str, DBUS_BIG_ENDIAN, pos, &pos) == 0x12345678)
    _dbus_assert_not_reached ("demarshal failed");
  
  if (!_dbus_marshal_uint32 (&str, DBUS_LITTLE_ENDIAN, 0x12345678))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_uint32 (&str, DBUS_LITTLE_ENDIAN, pos, &pos) == 0x12345678)
    _dbus_assert_not_reached ("demarshal failed");

#ifdef DBUS_HAVE_INT64
  /* Marshal signed integers */
  if (!_dbus_marshal_int64 (&str, DBUS_BIG_ENDIAN, DBUS_INT64_CONSTANT (-0x123456789abc7)))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_int64 (&str, DBUS_BIG_ENDIAN, pos, &pos) == DBUS_INT64_CONSTANT (-0x123456789abc7))
    _dbus_assert_not_reached ("demarshal failed");

  if (!_dbus_marshal_int64 (&str, DBUS_LITTLE_ENDIAN, DBUS_INT64_CONSTANT (-0x123456789abc7)))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_int64 (&str, DBUS_LITTLE_ENDIAN, pos, &pos) == DBUS_INT64_CONSTANT (-0x123456789abc7))
    _dbus_assert_not_reached ("demarshal failed");
  
  /* Marshal unsigned integers */
  if (!_dbus_marshal_uint64 (&str, DBUS_BIG_ENDIAN, DBUS_UINT64_CONSTANT (0x123456789abc7)))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_uint64 (&str, DBUS_BIG_ENDIAN, pos, &pos) == DBUS_UINT64_CONSTANT (0x123456789abc7))
    _dbus_assert_not_reached ("demarshal failed");
  
  if (!_dbus_marshal_uint64 (&str, DBUS_LITTLE_ENDIAN, DBUS_UINT64_CONSTANT (0x123456789abc7)))
    _dbus_assert_not_reached ("could not marshal signed integer value");
  if (!_dbus_demarshal_uint64 (&str, DBUS_LITTLE_ENDIAN, pos, &pos) == DBUS_UINT64_CONSTANT (0x123456789abc7))
    _dbus_assert_not_reached ("demarshal failed");
#endif /* DBUS_HAVE_INT64 */
  
  /* Marshal strings */
  tmp1 = "This is the dbus test string";
  if (!_dbus_marshal_string (&str, DBUS_BIG_ENDIAN, tmp1))
    _dbus_assert_not_reached ("could not marshal string");
  tmp2 = _dbus_demarshal_string (&str, DBUS_BIG_ENDIAN, pos, &pos);
  if (!strcmp (tmp1, tmp2) == 0)
    _dbus_assert_not_reached ("demarshal failed");
  dbus_free (tmp2);

  tmp1 = "This is the dbus test string";
  if (!_dbus_marshal_string (&str, DBUS_LITTLE_ENDIAN, tmp1))
    _dbus_assert_not_reached ("could not marshal string");
  tmp2 = _dbus_demarshal_string (&str, DBUS_LITTLE_ENDIAN, pos, &pos);
  if (!strcmp (tmp1, tmp2) == 0)
    _dbus_assert_not_reached ("demarshal failed");
  dbus_free (tmp2);

  /* Marshal signed integer arrays */
  if (!_dbus_marshal_int32_array (&str, DBUS_BIG_ENDIAN, array1, 3))
    _dbus_assert_not_reached ("could not marshal integer array");
  if (!_dbus_demarshal_int32_array (&str, DBUS_BIG_ENDIAN, pos, &pos, &array2, &len))
    _dbus_assert_not_reached ("could not demarshal integer array");

  if (len != 3)
    _dbus_assert_not_reached ("Signed integer array lengths differ!\n");
  dbus_free (array2);

#ifdef DBUS_HAVE_INT64
  /* Marshal 64-bit signed integer arrays */
  if (!_dbus_marshal_int64_array (&str, DBUS_BIG_ENDIAN, array3, 3))
    _dbus_assert_not_reached ("could not marshal integer array");
  if (!_dbus_demarshal_int64_array (&str, DBUS_BIG_ENDIAN, pos, &pos, &array4, &len))
    _dbus_assert_not_reached ("could not demarshal integer array");

  if (len != 3)
    _dbus_assert_not_reached ("Signed integer array lengths differ!\n");
  dbus_free (array4);
#endif
  
  _dbus_string_free (&str);
  
      
  return TRUE;
}

#endif /* DBUS_BUILD_TESTS */
