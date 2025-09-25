/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"

/**
  @brief  Compare the two input buffer content
  @param  src   - Source buffer to be compared
  @dest   dest  - Destination buffer to be compared
  @return Zero if buffer content are equal, else non-zero
**/
int val_memcmp(void *src, void *dest, size_t len)
{
  return pal_memcmp(src, dest, len);
}

/**
  @brief  Copy src buffer content into dst
  @param  dst  - Pointer to the destination buffer
  @param  src  - Pointer to the source buffer
  @param  len  - Number of bytes in buffer to copy
  @return dst
**/
void *val_memcpy(void *dst, const void *src, size_t len)
{
  return pal_memcpy(dst, src, len);
}

/**
  @brief  Fill a buffer with a known specified input value
  @param  dst   - Pointer to the buffer to fill
  @param  value - Value to fill buffer with
  @param  count  - Number of bytes in buffer to fill
  @return None
**/
void *val_memset(void *dst, int val, size_t count)
{
  pal_memset(dst, val, count);
  return dst;
}

/**
  @brief  Appends the string pointed to by str2 to the
          end of the string pointed to by str1
  @param  str1  - Pointer of destination string
  @param  str2  - Pointer of string to be appended
  @param  output_buff_size - Size of str1 string
  @return Pointer of destination string
**/
char *val_strcat(char *str1, const char *str2, size_t output_buff_size)
{
    // Find the end of the first string
    size_t str1_len = 0;
    while (str1[str1_len] != '\0' && str1_len < output_buff_size - 1)
    {
        str1_len++;
    }

    // If str1 is already at the maximum buffer size, return str1
    if (str1_len >= output_buff_size - 1) {
        return str1;
    }

    // Append characters from str2 to str1, ensuring we don't exceed the buffer size
    size_t i = 0;
    while (str2[i] != '\0' && str1_len < output_buff_size - 1)
    {
        str1[str1_len] = str2[i];
        str1_len++;
        i++;
    }

    // Null-terminate the resulting string
    str1[str1_len] = '\0';

    return str1;
}

/**
  @brief  Compare two strings with size consideration
  @param  str1  - Pointer of first string
  @param  str2  - Pointer of second string
  @return Zero if strings are equal, else non-zero
**/
int val_strcmp(char *str1, char *str2)
{
    uint32_t ctr = 0;

    while (str1[ctr] == str2[ctr])
    {
        if (str1[ctr] == '\0' || str2[ctr] == '\0')
            break;
        ctr++;
    }
    if (str1[ctr] == '\0' && str2[ctr] == '\0')
        return 0;
    else
        return -1;
}

/**
 * @brief Copies the string pointed to by src (including the null terminator)
 *        to the buffer pointed to by dest.
 *
 * @param dest Pointer to the destination buffer.
 * @param src  Pointer to the null-terminated source string.
 * @return Pointer to the destination string dest.
 */
char *val_strcpy(char *dest, const char *src)
{
    char *ret = dest;

    while ((*dest++ = *src++));

    return ret;
}

/**
 * @brief Safely copies memory from source to destination, handling overlap.
 *
 * This function copies `n` bytes from the memory area pointed to by `src` to
 * the memory area pointed to by `dest`. It ensures correct behavior even when
 * the source and destination regions overlap, mimicking the behavior of the
 * standard `memmove` function.
 *
 * If `dest` is before `src`, it performs a forward copy.
 * If `dest` is after or overlaps `src`, it performs a backward copy to prevent
 * data corruption.
 *
 * @param dest Pointer to the destination memory area.
 * @param src  Pointer to the source memory area.
 * @param n    Number of bytes to copy.
 *
 * @return Pointer to the destination memory area (`dest`).
 */
void *val_memmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d == s || n == 0)
        return dest;

    if (d < s) {
        // Safe to copy forward
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        // Safe to copy backward
        for (size_t i = n; i-- > 0;)
            d[i] = s[i];
    }
    return dest;
}

/**
 * @brief       - Returns the physical address of the input virtual address
 * @param       - va: Virtual address of the memory to be converted
 * @return      - Physical address(PA or IPA)
**/
void *val_mem_virt_to_phys(void *va)
{
  return pal_mem_virt_to_phys(va);
}

#ifdef TARGET_LINUX
/**
 * @brief Map the given region into page table
 * @param mem_desc - memory addrs and attributes needed for page table mapping.
 * @return status
**/
uint32_t val_mem_map_pgt(memory_region_descriptor_t *mem_desc)
{
    (void)mem_desc;
    return VAL_SUCCESS;
}
#endif
