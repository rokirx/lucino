#ifndef LUCENE_UTIL_H
#define LUCENE_UTIL_H

#include "lucene.h"

#define LUCENE_IS_CHAR(c)  (\
             ( c >= 'a' && c <= 'z' ) ||         \
             ( c >= 'A' && c <= 'Z' ) ||         \
             c == 'ä' || c == 'ü' || c == 'ö' || \
             c == 'Ä' || c == 'Ü' || c == 'Ö' || c == 'ß' || \
             c == 'é' || c == 'á' )

#define LUCENE_IS_UPPER(c) ( ( c >= 'A' && c <= 'Z' ) || c == 'Ä' || c == 'Ö' || c == 'Ü' )

#define LUCENE_IS_DIGIT(c) ( c >= '0' && c <= '9' )

/**
 * Converts an integer to the numeric representation
 * to base 32. Up to 7 characters may be written to
 * buffer
 */
LUCENE_EXTERN void
lcn_itoa32 ( unsigned int i, char *buf );

/**
 * Converts a numeric representation to base 32 to an
 * integer. At most first 7 digits are evaluated.
 */
LUCENE_EXTERN unsigned int
lcn_atoi32 ( const char *buf );

/**
 * Converts a numeric representation to base 32 to an
 * integer. At most first 7 digits are evaluated.
 */
LUCENE_EXTERN unsigned int
lcn_atoi36 ( const char *buf );

/**
 * Compares two strings, character by character, and returns the
 * first position where the two strings differ from one another.
 *
 * @param s1 The first string to compare
 * @param s2 The second string to compare
 * @return The first position where the two strings differ.
 */
int
lcn_string_difference ( const char *s1, const char *s2 );


#endif /* LUCENE_UTIL_H */
