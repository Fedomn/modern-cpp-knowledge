#include <gtest/gtest.h>

/* Bits for different SQL modes modes (including ANSI mode) */
#define MODE_REAL_AS_FLOAT           1
#define MODE_PIPES_AS_CONCAT         2
#define MODE_ANSI_QUOTES             4
#define MODE_IGNORE_SPACE            8
#define MODE_NOT_USED                16
#define MODE_ONLY_FULL_GROUP_BY      32
#define MODE_NO_UNSIGNED_SUBTRACTION 64
#define MODE_NO_DIR_IN_CREATE        128
#define MODE_ANSI                    262144L
#define MODE_NO_AUTO_VALUE_ON_ZERO   (MODE_ANSI * 2)
#define MODE_NO_BACKSLASH_ESCAPES    (MODE_NO_AUTO_VALUE_ON_ZERO * 2)
#define MODE_STRICT_TRANS_TABLES     (MODE_NO_BACKSLASH_ESCAPES * 2)
#define MODE_STRICT_ALL_TABLES       (MODE_STRICT_TRANS_TABLES * 2)
/*
 * NO_ZERO_DATE, NO_ZERO_IN_DATE and ERROR_FOR_DIVISION_BY_ZERO modes are
 * removed in 5.7 and their functionality is merged with STRICT MODE.
 * However, For backward compatibility during upgrade, these modes are kept
 * but they are not used. Setting these modes in 5.7 will give warning and
 * have no effect.
 */
#define MODE_NO_ZERO_IN_DATE            (MODE_STRICT_ALL_TABLES * 2)
#define MODE_NO_ZERO_DATE               (MODE_NO_ZERO_IN_DATE * 2)
#define MODE_INVALID_DATES              (MODE_NO_ZERO_DATE * 2)
#define MODE_ERROR_FOR_DIVISION_BY_ZERO (MODE_INVALID_DATES * 2)
#define MODE_TRADITIONAL                (MODE_ERROR_FOR_DIVISION_BY_ZERO * 2)
#define MODE_HIGH_NOT_PRECEDENCE        (1ULL << 29)
#define MODE_NO_ENGINE_SUBSTITUTION     (MODE_HIGH_NOT_PRECEDENCE * 2)
#define MODE_PAD_CHAR_TO_FULL_LENGTH    (1ULL << 31)
/*
  If this mode is set the fractional seconds which cannot fit in given fsp will
  be truncated.
*/
#define MODE_TIME_TRUNCATE_FRACTIONAL (1ULL << 32)

#define MODE_LAST (1ULL << 33)

#define MODE_ALLOWED_MASK                                                                                               \
  (MODE_REAL_AS_FLOAT | MODE_PIPES_AS_CONCAT | MODE_ANSI_QUOTES | MODE_IGNORE_SPACE | MODE_NOT_USED |                   \
   MODE_ONLY_FULL_GROUP_BY | MODE_NO_UNSIGNED_SUBTRACTION | MODE_NO_DIR_IN_CREATE | MODE_ANSI |                         \
   MODE_NO_AUTO_VALUE_ON_ZERO | MODE_NO_BACKSLASH_ESCAPES | MODE_STRICT_TRANS_TABLES | MODE_STRICT_ALL_TABLES |         \
   MODE_NO_ZERO_IN_DATE | MODE_NO_ZERO_DATE | MODE_INVALID_DATES | MODE_ERROR_FOR_DIVISION_BY_ZERO | MODE_TRADITIONAL | \
   MODE_HIGH_NOT_PRECEDENCE | MODE_NO_ENGINE_SUBSTITUTION | MODE_PAD_CHAR_TO_FULL_LENGTH | MODE_TIME_TRUNCATE_FRACTIONAL)

/*
  We can safely ignore and reset these obsolete mode bits while replicating:
*/
#define MODE_IGNORED_MASK                         \
  (0x00100 |  /* was: MODE_POSTGRESQL          */ \
   0x00200 |  /* was: MODE_ORACLE              */ \
   0x00400 |  /* was: MODE_MSSQL               */ \
   0x00800 |  /* was: MODE_DB2                 */ \
   0x01000 |  /* was: MODE_MAXDB               */ \
   0x02000 |  /* was: MODE_NO_KEY_OPTIONS      */ \
   0x04000 |  /* was: MODE_NO_TABLE_OPTIONS    */ \
   0x08000 |  /* was: MODE_NO_FIELD_OPTIONS    */ \
   0x10000 |  /* was: MODE_MYSQL323            */ \
   0x20000 |  /* was: MODE_MYSQL40             */ \
   0x10000000 /* was: MODE_NO_AUTO_CREATE_USER */ \
  )

TEST(MySQLTest, Const)  // NOLINT
{
  auto a = MODE_REAL_AS_FLOAT | MODE_PIPES_AS_CONCAT | MODE_ANSI_QUOTES | MODE_IGNORE_SPACE | MODE_ONLY_FULL_GROUP_BY |
           MODE_ANSI | MODE_STRICT_TRANS_TABLES | MODE_STRICT_ALL_TABLES | MODE_NO_ZERO_IN_DATE | MODE_NO_ZERO_DATE |
           MODE_ERROR_FOR_DIVISION_BY_ZERO | MODE_TRADITIONAL | MODE_NO_ENGINE_SUBSTITUTION;
  auto b = MODE_ONLY_FULL_GROUP_BY | MODE_STRICT_TRANS_TABLES | MODE_NO_ZERO_IN_DATE | MODE_NO_ZERO_DATE |
           MODE_ERROR_FOR_DIVISION_BY_ZERO | MODE_NO_ENGINE_SUBSTITUTION;
  EXPECT_EQ(a, 1306787887);
  EXPECT_EQ(b, 1168113696);
}