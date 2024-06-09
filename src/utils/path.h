#pragma once
#ifndef __PATH_H__
#define __PATH_H__

/**
 * @brief Creates a normalized version of the path.
 *
 * This function creates a normalized version of the path within the specified
 * buffer. This function will not write out more than the specified buffer can
 * contain. However, the generated string is always null-terminated - even if
 * not the whole path is written out. The function returns the total number of
 * characters the complete buffer would have, even if it was not written out
 * completely. The path may be the same memory address as the buffer.
 *
 * The following will be true for the normalized path:
 * 1) "../" will be resolved.
 * 2) "./" will be removed.
 * 3) double separators will be fixed with a single separator.
 * 4) separator suffixes will be removed.
 *
 * @param path The path which will be normalized.
 * @param buffer The buffer where the new path is written to.
 * @param buffer_size The size of the buffer.
 * @return The size which the complete normalized path has if it was not
 * truncated.
 */
size_t
sy_path_normalize(const char *path, char *buffer, size_t buffer_size);

/**
 * @brief Generates a relative path based on a base.
 *
 * This function generates a relative path based on a base path and another
 * path. It determines how to get to the submitted path, starting from the
 * base directory. The result will be written to a buffer, which might be
 * truncated if the buffer is not large enough to hold the full path. However,
 * the truncated result will always be null-terminated. The returned value is
 * the amount of characters which the resulting path would take if it was not
 * truncated (excluding the null-terminating character).
 *
 * @param base_directory The base path from which the relative path will
 * start.
 * @param path The target path where the relative path will point to.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the full path.
 */
size_t 
sy_path_get_relative(const char *base_directory, const char *path, char *buffer, size_t buffer_size);

/**
 * @brief Generates an absolute path based on a base.
 *
 * This function generates an absolute path based on a base path and another
 * path. It is guaranteed to return an absolute path. If the second submitted
 * path is absolute, it will override the base path. The result will be
 * written to a buffer, which might be truncated if the buffer is not large
 * enough to hold the full path. However, the truncated result will always be
 * null-terminated. The returned value is the amount of characters which the
 * resulting path would take if it was not truncated (excluding the
 * null-terminating character).
 *
 * @param base The absolute base path on which the relative path will be
 * applied.
 * @param path The relative path which will be applied on the base path.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the new absolute path.
 */
size_t 
sy_path_get_absolute(const char *base, const char *path, char *buffer, size_t buffer_size);

/**
 * @brief Joins two paths together.
 *
 * This function generates a new path by combining the two submitted paths. It
 * will remove double separators, and unlike cwk_sy_path_get_absolute it permits
 * the use of two relative paths to combine. The result will be written to a
 * buffer, which might be truncated if the buffer is not large enough to hold
 * the full path. However, the truncated result will always be
 * null-terminated. The returned value is the amount of characters which the
 * resulting path would take if it was not truncated (excluding the
 * null-terminating character).
 *
 * @param path_a The first path which comes first.
 * @param path_b The second path which comes after the first.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the full, combined path.
 */
size_t
sy_path_join(const char *path_a, const char *path_b, char *buffer, size_t buffer_size);

/**
 * @brief Joins multiple paths together.
 *
 * This function generates a new path by joining multiple paths together. It
 * will remove double separators, and unlike cwk_sy_path_get_absolute it permits
 * the use of multiple relative paths to combine. The last path of the
 * submitted string array must be set to NULL. The result will be written to a
 * buffer, which might be truncated if the buffer is not large enough to hold
 * the full path. However, the truncated result will always be
 * null-terminated. The returned value is the amount of characters which the
 * resulting path would take if it was not truncated (excluding the
 * null-terminating character).
 *
 * @param paths An array of paths which will be joined.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the full, combined path.
 */
size_t
sy_path_join_multiple(const char **paths, char *buffer, size_t buffer_size);

/**
 * @brief Determine whether the path is relative or not.
 *
 * This function checks whether the path is a relative path or not. A path is
 * considered to be relative if the root does not end with a separator.
 *
 * @param path The path which will be checked.
 * @return Returns true if the path is relative or false otherwise.
 */
int32_t
sy_path_is_relative(const char *path);

/**
 * @brief Determine whether the path is absolute or not.
 *
 * This function checks whether the path is an absolute path or not. A path is
 * considered to be absolute if the root ends with a separator.
 *
 * @param path The path which will be checked.
 * @return Returns true if the path is absolute or false otherwise.
 */
int32_t
sy_path_is_absolute(const char *path);

int32_t 
path_is_directory(const char *path);

char *
sy_path_get_current_directory(char *buffer, size_t buffer_size);

int32_t
sy_path_is_root(const char *path);

size_t
sy_path_get_filename(const char *path, char *buffer, size_t buffer_size);

size_t
sy_path_get_directory_path(const char *path, char *buffer, size_t buffer_size);

int32_t
sy_path_exist(const char *path);

#endif