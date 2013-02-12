/**
 * @file lcn_error_codes.h
 * @brief Lucene error codes.
 */

#if defined(LCN_ERROR_BUILD_ARRAY) || !defined(LCN_ERROR_ENUM_DEFINED)

#include <apr.h>
#include <apr_errno.h>
#include "lucene.h"

BEGIN_C_DECLS

#if defined(LCN_ERROR_BUILD_ARRAY)

#define LCN_ERROR_START static const err_defn error_table[] = {
#define LCN_ERRDEF(num, offset, str) { num, str },
#define LCN_ERROR_END { 0, NULL } };

#elif !defined(LCN_ERROR_ENUM_DEFINED)

#define LCN_ERROR_START typedef enum lcn_errno_t {
#define LCN_ERRDEF(num, offset, str)  num = (APR_OS_START_USERERR + offset),
#define LCN_ERROR_END } lcn_errno_t;

#define LCN_ERROR_ENUM_DEFINED

#endif

LCN_ERROR_START

LCN_ERRDEF( LCN_ERR_NOT_REGULAR_FILE,
            1, "File not found or is not regular file" )

LCN_ERRDEF( LCN_ERR_CF_SUB_FILE_NOT_FOUND,
            2, "A sub-file of compound_file was not found" )

LCN_ERRDEF( LCN_ERR_READ_PAST_EOF,
            3, "Read past end of file" )

LCN_ERRDEF( LCN_ERR_NULL_PTR,
            4, "A pointer was null (as a param of a function) which may not be null" )

LCN_ERRDEF( LCN_ERR_INVALID_BV_SIZE,
            5, "Invalid size of a bit vector was read from a file" )

LCN_ERRDEF( LCN_ERR_INVALID_BV_OP,
            6, "An operation on two bit vectors is illegal because of different sizes" )

LCN_ERRDEF( LCN_ERR_UNSUPPORTED_OPERATION,
            7, "Operation not supported by the object" )

LCN_ERRDEF( LCN_ERR_CANNOT_OPEN_DIR,
            8, "File System Directory could not be opened" )

LCN_ERRDEF( LCN_ERR_TERM_OUT_OF_ORDER,
            9, "Term out of order" )

LCN_ERRDEF( LCN_ERR_FREQ_POINTER_OUT_OF_ORDER,
            10, "Frequency pointer out of order" )

LCN_ERRDEF( LCN_ERR_PROX_POINTER_OUT_OF_ORDER,
            11, "Proximity pointer out of order" )

LCN_ERRDEF( LCN_ERR_DOCUMENT_NO_SUCH_FIELD,
            12, "Document has no such field" )

LCN_ERRDEF( LCN_ERR_RAM_FILE_NOT_FOUND,
            13, "A Ram-Directory-File cannot be opened" )

LCN_ERRDEF( LCN_ERR_FIELD_NOT_FOUND,
            14, "Requested field was not found in field infos" )

LCN_ERRDEF( LCN_ERR_INDEX_OUT_OF_RANGE,
            15, "Array or list index out of range" )

LCN_ERRDEF( LCN_ERR_SEGMENT_INFOS_UNKNOWN_FILE_FORMAT,
            16, "Unknown format of segment infos file" )

LCN_ERRDEF( LCN_ERR_TERM_INFOS_READER_EMPTY,
            17, "Term infos reader does not contain terms" )

LCN_ERRDEF( LCN_ERR_FIELD_INFO_OMIT_NORMS_ON_UNINDEXED,
            18, "Trying to omit norms on unindexed field" )

LCN_ERRDEF( LCN_ERR_FIELD_INFO_STORE_TERM_VECTOR_ON_UNINDEXED,
            19, "Trying to store a term vector on unindexed field" )

LCN_ERRDEF( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM,
            20, "Could not scan to term in a term enumeration" )

LCN_ERRDEF( LCN_ERR_TERM_INFOS_READER_NO_TERM_AT_POS,
            21, "Term infos reader could not find a term at the given position" )

LCN_ERRDEF( LCN_ERR_TERM_INFOS_READER_NO_POS_FOR_TERM,
            22, "Term infos reader could not calculate position of term" )

LCN_ERRDEF( LCN_ERR_ACCESS_DELETED_DOCUMENT,
            23, "Attempt to access a deleted document" )

LCN_ERRDEF( LCN_ERR_TOKEN_STREAM_EOS,
            24, "End of Token-Stream reached" )

LCN_ERRDEF( LCN_ERR_NORMS_NOT_FOUND,
            25, "No norms exist for given field" )

LCN_ERRDEF( LCN_ERR_TERM_DOCS_TERM_IS_NULL,
            26, "Term was not initialized in MultiTermDocs while getting internal term_docs" )

LCN_ERRDEF( LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION,
            27, "Inconsistent field definition in indexes/segments to be merged" )

LCN_ERRDEF( LCN_ERR_ADDING_DUPLICATE_FIELD,
            28, "Trying to add a second field to a document which already contains a field with the same name" )

LCN_ERRDEF( LCN_ERR_FIELD_ANALYZER_NOT_INITIALIZED,
            29, "Trying to get an uninitialized analyzer of a field" )

LCN_ERRDEF( LCN_ERR_FIELD_UNKNOWN_COPY_VALUE_FLAG,
            30, "Unknown flag for copy_value in lcn_field_create_*" )

LCN_ERRDEF( LCN_ERR_ITERATOR_NO_NEXT,
            31, "No Elementes left for iterating" )

LCN_ERRDEF( LCN_ERR_ITERATOR_ACCESS_BEFORE_NEXT,
            32, "Attempt to access Member before calling next" )

LCN_ERRDEF( LCN_ERR_DOCUMENT_FIELD_IS_BINARY,
            33,
            "Attempt to access a binary document-field by "
            "lcn_document_get" )

LCN_ERRDEF( LCN_ERR_SCORER_NOT_ENOUGH_SUBSCORERS,
            34,
            "There must be at least 2 sub-scorers" )

LCN_ERRDEF( LCN_ERR_NO_SUCH_DOC,
            35,
            "Attempt to skip to not existing document" )

LCN_ERRDEF( LCN_ERR_INVALID_ARGUMENT,
            36,
            "Invalid input for method" )

LCN_ERRDEF( LCN_ERR_DOCS_OUT_OF_ORDER,
            37,
            "Docs out of order" )

LCN_ERRDEF( LCN_ERR_MISSING_SIZE_PARAM,
            38,
            "Missing fixed size parameter in adding fixed size field" )

LCN_ERRDEF( LCN_ERR_EMPTY_QUERY,
            39,
            "Attempt to trigger a search on an empty query" )

LCN_ERRDEF( LCN_ERR_SCAN_ENUM_NO_MATCH,
            40,
            "lcn_term_infos_reader_term_enum positions enum not at an exact match")

LCN_ERRDEF( LCN_ERR_DOCUMENT_DUMP_FMT,
            41,
            "Noticed syntax error when parsing document dump" )

LCN_ERRDEF( LCN_ERR_DOCUMENT_DUMP_NO_SUCH_ANALYZER,
            42,
            "An analyzer specified in a document_dump was not provided")

LCN_ERRDEF( LCN_ERR_INDEX_WRITER_ADDING_EMTY_DOCUMENT,
            43,
            "Attemt to add a document without fields to an index")

LCN_ERRDEF( LCN_ERR_IO,
            44,
            "An error while writing to a stream via printf-funcions occured")

LCN_ERRDEF( LCN_ERR_TERM_INFOS_READER_NO_TIS_FILE,
            45, "Index does not contain any indexed fields and therefore no .tis-file" )

LCN_ERRDEF( LCN_ERR_INVALID_FILE_VERSION_NUMBER,
            46, "Index file has a not supported version number" )

LCN_ERRDEF( LCN_ERR_QUERY_PARSER_SYNTAX_ERROR,
            47, "Syntax error in the lucene query")

LCN_ERRDEF( LCN_ERR_FS_FIELD_INCONSISTENT_OFFSET,
            48, "Inconsistent offsets in multi fs field")

LCN_ERRDEF( LCN_ERR_GROUP_INDEX_OUT_OF_RANGE,
            49, "Group index in hits points to a group member not cached in the hit group")

LCN_ERRDEF( LCN_ERR_SETTING_FROZEN_FIELD_TYPE,
            50, "Trying to set a value on a frozen field type" )

LCN_ERRDEF( LCN_ERR_CF_DATA_COPY_NOT_COMPLETE,
            51, "Compound file data not full copied" )

LCN_ERRDEF( LCN_ERR_CF_DATA_DIFFERENT_OFFSETS,
            52, "Compound file data offset does not match the original file length" )

LCN_ERRDEF( LCN_ERR_STREAM_CLOSED,
            53, "Tried to read closed stream" )

LCN_ERRDEF( LCN_ERR_METHOD_NOT_IMPLEMENTED,
            54, "The called Method was not implemented. Watch error Log for more ditails." )

LCN_ERRDEF( LCN_ERR_SEGMENT_INFOS_INVALID_SEGMENTS_FILE_NAME,
            55, "The name of a segments file has invalid format" )

LCN_ERRDEF( LCN_ERR_ALREADY_CLOSED,
            56, "Trying to operate on closed resource" )

LCN_ERRDEF( LCN_ERR_INDEX_NOT_FOUND,
            57, "No segments* file found in directory" )

LCN_ERRDEF( LCN_PRIORITY_QUEUE_MAX_SIZE_EXCEEDED,
            58, "Priority queue max size exceeded." )

LCN_ERROR_END


#undef LCN_ERROR_START
#undef LCN_ERRDEF
#undef LCN_ERROR_END

END_C_DECLS

#endif /* defined(LCN_ERROR_BUILD_ARRAY) || !defined(LCN_ERROR_ENUM_DEFINED) */

